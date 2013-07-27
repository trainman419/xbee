/* xcsh2.cpp
 *
 * New XBee shell
 *
 * Author: Austin Hendrix
 */


#include <serial/serial.h>

#include <string>
#include <list>

#include <stdlib.h>
#include <ctype.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "xbee_api.h"
#include "prefix_map.h"
#include "xbee_at_cmd.h"

// commands:
//  remote mode: switch to remote AT commands. pushes address onto remote stack
//  exit: leave current remote node; or exit completely
//
// API frames:
//  see xbee_api.h
//
//

// design:
//  data flow:
//   prompt> user commands are a single line; parsed as whitespace-delimited
//    tokens. The first token is the top-level command group; beyond that, each
//    group is responsible for its own completion/help etc
//
//    The prompt should have tab-completion and help at various levels
//     - similar to cisco/juniper/dell CLI
//
//     commands will be stored in a tree of prefix maps;
//      there will be an object for each command at each level; each object will
//      have methods for:
//       - tab complete suggestions
//       - help text
//       - subcommand(s) help by prefix
//       - subcommand call
//      call points will be:
//       - tab-complete
//       - command entered
//   asynchroous data:
//    various commands can result in delayed or asynchronous data delivered to
//    the user; parse and format this data for display, without interrupting
//    the user's prompt.
//

xbsh_state::xbsh_state(std::string port, int baud ) :
   serial(port, baud, serial::Timeout(10, 100, 0)),
   read_thread_done(false), debug(1), read_thread(boost::ref(*this)),
   partial(), remotes(), received_frames(), received_frames_mutex(),
   received_frames_cond()
{
}

xbsh_state::~xbsh_state() {
   read_thread_done = true;
   read_thread.join();
}

void xbsh_state::send_packet(packet p) {
   if( debug ) {
      printf("TX: ");
      for( int i=0; i<p.sz; i++ ) {
         printf("%02X ", p.data[i]);
      }
      printf("\n");
   }
   serial.write(p.data, p.sz);
}

void xbsh_state::send_AT(std::string at_str) {
   send_AT(at_str, std::vector<uint8_t>());
}

void xbsh_state::send_AT(std::string at_str, std::vector<uint8_t> data) {
   if( at_str.length() != 2 ) {
      fprintf(stderr, "Error: bad AT command: %s", at_str.c_str());
      return;
   }
   int d_sz = at_str.length() + data.size();
   char * d = (char*)malloc(d_sz + 1);
   memcpy(d, at_str.c_str(), 2);
   for( int i=0; i<data.size(); i++ ) {
      d[i+2] = data[i];
   }

   packet p;
   if( remotes.size() == 0 ) {
      p = at(d, d_sz);
   } else {
      xbee_net net;
      net.c_net[0] = 0xFF;
      net.c_net[1] = 0xFE;
      p = remote_at(remotes.back(), net, d, d_sz);
   }
   send_packet(p);
   free(d);
}

void xbsh_state::hardreset() {
   serial.setDTR(false);
   serial.setRTS(false);

   sleep(1);

   serial.setDTR(true);
   serial.setRTS(true);
}

void xbsh_state::operator()() {
   std::vector<uint8_t> raw_data;

   while(!read_thread_done) {
      // temporary buffer for incoming data
      std::vector<uint8_t> tmp_data;
      serial.read(tmp_data, 64);
      if( tmp_data.size() > 0 ) {
         if( debug ) {
            printf("RX:    ");
            for( int i=0; i<tmp_data.size(); i++ ) {
               printf(" %02X", tmp_data[i]);
            }
            printf("\n");
         }
      }

      // append tmp_data to data
      raw_data.insert(raw_data.end(), tmp_data.begin(), tmp_data.end());

      // eat any starting garbage bytes
      int bad_cnt = 0;
      std::vector<uint8_t>::iterator itr;
      for( itr = raw_data.begin(); 
            itr != raw_data.end() && *itr != 0x7E;
            ++itr, ++bad_cnt );
      raw_data.erase(raw_data.begin(), itr);
      if( debug && bad_cnt > 0 ) {
         printf("Ate %d bad leading bytes\n", bad_cnt);
      }

      if( raw_data.size() > 3 && raw_data[0] == 0x7E ) {
         std::vector<uint8_t> data;
         data.reserve(raw_data.size());
         // un-escape raw data
         int i=0;
         data.push_back(raw_data[0]);
         for( i=1; i<raw_data.size() && raw_data[i] != 0x7E; ++i ) {
            if(raw_data[i] == 0x7D) {
               ++i;
               data.push_back(raw_data[i] ^ 0x20);
            } else {
               data.push_back(raw_data[i]);
            }
         }
         // eat bytes off the front of our incoming buffer
         raw_data.erase(raw_data.begin(), raw_data.begin()+i);

         if( debug ) {
            printf("Packet:");
            for( i=0; i<data.size(); ++i ) {
               printf(" %02X", data[i]);
            }
            printf("\n");
         }

         int len = data[2] | (data[1] << 8);
         // TODO: only consume the first len bytes, or wait for more if the
         //  buffer has less than len bytes
         if( len + 4 <= data.size() ) {
            //std::vector<uint8_t> packet_data(
            uint8_t checksum = data[3 + len];
            uint8_t sum = 0;
            for( int i=0; i<len; ++i ) {
               sum += data[3+i];
            }
            sum = 0xFF - sum;
            if( sum == checksum ) {
               uint8_t type = data[3];
               uint8_t id = data[4];
               switch(type) {
                  case API_AT_RESP:
                     {
                        if( len < 5 ) {
                           printf("API response too short: %d\n", len);
                           // TODO: ?
                        } else {
                           std::string command;
                           command.push_back(data[5]);
                           command.push_back(data[6]);
                           uint8_t status = data[7];
                           std::vector<uint8_t> d(len - 5);
                           for( int i=0; i<len-5; i++ ) {
                              d[i] = data[i+8];
                           }
                           // lock output array and append
                           api_frame * f = new api_frame(type, id, status, 
                                 command, d);
                           {
                              boost::mutex::scoped_lock lock(
                                    received_frames_mutex);
                              received_frames.push_back(f);
                           }
                           received_frames_cond.notify_one();
                        }
                     }
                     break;
                  case API_REMOTE_AT_RESP:
                     if( len < 15 ) {
                        printf("Remote API response too short: %d\n", len);
                        // TODO: ?
                     } else {
                        xbee_addr source;
                        source.c_addr[0] = data[12];
                        source.c_addr[1] = data[11];
                        source.c_addr[2] = data[10];
                        source.c_addr[3] = data[9];
                        source.c_addr[4] = data[8];
                        source.c_addr[5] = data[7];
                        source.c_addr[6] = data[6];
                        source.c_addr[7] = data[5];
                        xbee_net net;
                        net.c_net[0] = data[14];
                        net.c_net[1] = data[13];
                        std::string command;
                        command.push_back(data[15]);
                        command.push_back(data[16]);
                        uint8_t status = data[17];
                        std::vector<uint8_t> d(len - 15);
                        for( int i=0; i<len-15; i++ ) {
                           d[i] = data[i+18];
                        }
                        // lock output array and append
                        api_frame * f = new api_remote_frame(type, id,
                              status, command, source, net, d);
                        {
                           boost::mutex::scoped_lock lock(
                                 received_frames_mutex);
                           received_frames.push_back(f);
                        }
                        received_frames_cond.notify_one();
                     }
                     break;
                  case API_IO_RX:
                     if( len < 16 ) {
                        printf("Remote sample too short: %d\n", len);
                     } else {
                        xbee_addr source;
                        source.c_addr[0] = data[11];
                        source.c_addr[1] = data[10];
                        source.c_addr[2] = data[9];
                        source.c_addr[3] = data[8];
                        source.c_addr[4] = data[7];
                        source.c_addr[5] = data[6];
                        source.c_addr[6] = data[5];
                        source.c_addr[7] = data[4];
                        xbee_net net;
                        net.c_net[0] = data[13];
                        net.c_net[1] = data[12];
                        uint8_t options = data[14];
                        std::vector<uint8_t> d(len - 12);
                        for( int i=0; i<len-12; i++ ) {
                           d[i] = data[i+15];
                        }
                        io_sample sample(d);
                        printf("Remote sample from %s\n", 
                              print_addr(source).c_str());
                        BOOST_FOREACH( const io_sample::digital di, 
                              sample.get_digital() ) {
                           printf("  DIO%d: %d\n", di.channel, di.data);
                        }
                        BOOST_FOREACH( const io_sample::analog a,
                              sample.get_analog() ) {
                           printf("  AD%d: %d\n", a.channel, a.data);
                        }
                     }
                     break;
                  default:
                     printf("Unknown response type %X\n", type);
                     break;
               }
            } else {
               printf("Checksum failed\n");
               // eat until next start byte
               int bad_cnt = 0;
               std::vector<uint8_t>::iterator itr = raw_data.begin();
               ++itr;
               for( ;
                     itr != raw_data.end() && *itr != 0x7E;
                     ++itr, ++bad_cnt );
               raw_data.erase(raw_data.begin(), itr);
               if( debug && bad_cnt > 0 ) {
                  printf("Ate %d bytes\n", bad_cnt);
               }
            }
         } else {
            printf("Packet length mismatch\n");
            // eat until next start byte
            int bad_cnt = 0;
            std::vector<uint8_t>::iterator itr = raw_data.begin();
            ++itr;
            for( ;
                  itr != raw_data.end() && *itr != 0x7E;
                  ++itr, ++bad_cnt );
            raw_data.erase(raw_data.begin(), itr);
            if( debug && bad_cnt > 0 ) {
               printf("Ate %d bytes\n", bad_cnt);
            }
         }
      } else {
         // unknown packet type
         if( raw_data.size() > 3 ) {
            printf("Got unknown packet type: %02X\n", raw_data[0]);
         }
      }
   }
}

api_frame * xbsh_state::read_AT() {
   boost::mutex::scoped_lock lock(received_frames_mutex);
   received_frames_cond.timed_wait(lock, boost::posix_time::seconds(10));
   if( received_frames.size() > 0 ) {
      api_frame * res = received_frames.front();
      received_frames.pop_front();
      return res;
   }
   return NULL;
}

std::string api_frame::get_error() {
   std::string err;
   switch(status) {
      case 0:
         err = "No Error??";
         break;
      case 1:
         err = "ERROR";
         break;
      case 2:
         err = "Invalid Command";
         break;
      case 3:
         err = "Invalid Parameter";
         break;
      case 4:
         err = "Tx Failure";
         break;
      default:
         err = "Unknown error";
         break;
   }
   return err;
}

std::string api_frame::to_string() {
   std::string res;
   for( int i=0; i<data.size(); i++ ) {
      char tmp[6];
      sprintf(tmp, "%02X, ", data[i]);
      res += std::string(tmp);
   }
   return res;
}

/*
std::string api_remote_frame::to_string() {
}
*/

// split a string into whitespace-separated parts
std::list<std::string> parts(std::string line) {
   const int l = line.length();
   int start = 0;
   int end = 0;
   std::list<std::string> result;
   while(end < l) {
      while(start < l && isspace(line[start])) start++;
      end = start;
      while(end < l && !isspace(line[end])) end++;
      if( start < end ) {
         result.push_back(line.substr(start, end-start));
      }
      start = end;
   }
   return result;
}

command * commands;

// get tab-completions
char ** xbsh_attempt_completion_func(const char * text, int start, int end) {

   std::list<std::string> p = parts(std::string(rl_line_buffer, 
            rl_line_buffer+start));

   // no more tab-complete:
   rl_attempted_completion_over = 1;

   command * c = commands;
   for( std::list<std::string>::iterator itr = p.begin();
         c && itr != p.end(); 
         itr++ ) {
      c = c->get_subcommand(*itr);
   }

   if( c ) {
      std::list<std::string> completions = c->get_completions(text);

      if( completions.size() == 1 ) {
         char ** result = (char**)malloc(sizeof(char*) * 2);
         std::string comp = completions.front();
         int len = comp.length();
         result[0] = (char*)malloc(sizeof(char)*(len+1));
         memcpy(result[0], comp.c_str(), len);
         result[0][len] = 0;

         result[1] = 0;
         return result;
      } else if( completions.size() > 1 ) {
         char ** result = 
            (char**)malloc(sizeof(char*) * (completions.size()+2));

         result[0] = (char*)malloc(strlen(text)+1);
         memcpy(result[0], text, strlen(text)+1);

         int i = 1;
         BOOST_FOREACH( std::string comp, completions ) {
            result[i] = (char*)malloc(sizeof(char) * (comp.length()+1));
            memcpy(result[i], comp.c_str(), comp.length()+1);
            i++;
         }
         result[i] = NULL;

         return result;
      } else {
         return NULL;
      }
   } else {
      // nothing to get completions for
      return NULL;
   }
}



int main(int argc, char ** argv) { 
   // initialize readline
   rl_attempted_completion_function = xbsh_attempt_completion_func;

   using_history();

   // initialize command tree:
   commands = setup_commands();

   std::string port;
   if( argc > 1 ) {
      port = argv[1];
   } else {
      port = "/dev/ttyUSB0";
   }

   int baud = 9600;
   if( argc > 2 ) {
      sscanf(argv[2], "%d", &baud);
   }

   xbsh_state * state;
   try {
      state = new xbsh_state(port, baud);
   } catch( serial::PortNotOpenedException & e ) {
      printf("Error opening serial port: %s\n", e.what());
      return -1;
   } catch( serial::IOException & e ) {
      printf("Error opening serial port: %s\n", e.what());
      return -2;
   }

   // readline main loop
   char * line;
   while( (line = readline("xbsh> ")) ) {
      std::list<std::string> line_parts = parts(line);
      std::string arg;
      if( line_parts.size() > 0 ) {
         arg = line_parts.back();
         line_parts.pop_back();
      }
      command * cmd = commands;
      BOOST_FOREACH( std::string part, line_parts ) {
         if(cmd) {
            cmd = cmd->get_subcommand(part);
         }
      }
      if( cmd ) {
         if( cmd->get_subcommand(arg) ) {
            cmd = cmd->get_subcommand(arg);
            arg = "";
         }
         add_history(line);
         int r = cmd->run(state, arg);
         if( r ) printf("Error: %d\n", r);
      } else {
         printf("Unknown command %s\n", line);
      }
      free(line);
   }
   printf("\n");
   return 0;
}
