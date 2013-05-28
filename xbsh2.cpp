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
  // TODO: spend more time choosing timeouts
  serial(port, baud, serial::Timeout(1000, 1000, 0))
{
}

void xbsh_state::send_packet(packet p) {
   // TODO
}

void xbsh_state::send_AT(std::string at_str, char * data, int data_len) {
   if( at_str.length() != 2 ) {
      fprintf(stderr, "Error: bad AT command: %s", at_str.c_str());
      return;
   }
   char * d = (char*)malloc(at_str.length() + data_len);
   memcpy(d, at_str.c_str(), 2);
   memcpy(d + 2, data, data_len);

   packet p;
   if( remotes.size() == 0 ) {
      p = at(d);
   } else {
      xbee_net net;
      net.c_net[0] = 0xFF;
      net.c_net[1] = 0xFE;
      p = remote_at(remotes.back(), net, d);
   }
   send_packet(p);
   free(d);
}

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
         result[0] = (char*)malloc(sizeof(char)*(len+2));
         memcpy(result[0], comp.c_str(), len);
         result[0][len] = ' ';
         result[0][len+1] = 0;

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
         printf("%d\n", r);
      } else {
         printf("Unknown command %s\n", line);
      }
      free(line);
   }
   printf("\n");
   return 0;
}
