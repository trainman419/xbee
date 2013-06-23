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
   serial(port, baud, serial::Timeout(100, 1000, 0)),
   read_thread_done(false), debug(0),
   read_thread(boost::ref(*this))
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

void xbsh_state::send_AT(std::string at_str, char * data, int data_len) {
   if( at_str.length() != 2 ) {
      fprintf(stderr, "Error: bad AT command: %s", at_str.c_str());
      return;
   }
   int d_sz = data_len + at_str.length();
   char * d = (char*)malloc(d_sz + 1);
   memcpy(d, at_str.c_str(), 2);
   memcpy(d + at_str.length(), data, data_len);
   d[d_sz] = 0; // append a null

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

void xbsh_state::operator()() {
   std::vector<uint8_t> data;

   while(!read_thread_done) {
      // temporary buffer for incoming data
      std::vector<uint8_t> tmp_data;
      serial.read(tmp_data, 64);
      if( tmp_data.size() > 0 ) {
         if( debug ) {
            printf("RX: ");
            for( int i=0; i<tmp_data.size(); i++ ) {
               printf("%02X ", tmp_data[i]);
            }
            printf("\n");
         }
      }

      // append tmp_data to data
      data.insert(data.end(), tmp_data.begin(), tmp_data.end());

      // TODO: eat any starting garbage bytes

      if( data.size() > 3 && data[0] == 0x7E ) {
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
                           // TODO: lock output array and append
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
                     {
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
                           // TODO: lock output array and append
                           api_frame * f = new api_remote_frame(type, id,
                                 status, command, source, net, d);
                           {
                              boost::mutex::scoped_lock lock(
                                    received_frames_mutex);
                              received_frames.push_back(f);
                           }
                           received_frames_cond.notify_one();
                        }
                     }
                     break;
                  default:
                     printf("Unknown response type %X\n", type);
                     break;
               }
               // eat len+4 bytes off the front of our incoming buffer
               data.erase(data.begin(), data.begin()+len+4);
            } else {
               printf("Checksum failed\n");
               // TODO: eat until next start byte
            }
         } else {
            printf("Packet length mismatch\n");
            // TODO: eat until next start byte
         }
      } else {
         // unknown packet type
         if( data.size() > 3 ) {
            printf("Got unknown packet type: %X\n", data[0]);
         }
      }
   }
}

api_frame * xbsh_state::read_AT() {
   boost::mutex::scoped_lock lock(received_frames_mutex);
   received_frames_cond.timed_wait(lock, boost::posix_time::seconds(1));
   if( received_frames.size() > 0 ) {
      api_frame * res = received_frames.front();
      received_frames.pop_front();
      return res;
   }
   return NULL;
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
