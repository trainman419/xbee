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

      if( completions.size() > 0 ) {
         char ** result = 
            (char**)malloc(sizeof(char*) * (completions.size()+2));

         int i = 0;

         if( completions.size() > 1 ) {
            result[0] = (char*)malloc(strlen(text)+1);
            memcpy(result[0], text, strlen(text)+1);
            i = 1;
         }

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

   // readline main loop
   char * line;
   while( line = readline("xbsh> ") ) {
      std::list<std::string> line_parts = parts(line);
      command * cmd = commands;
      BOOST_FOREACH( std::string part, line_parts ) {
         if(cmd) {
            cmd = cmd->get_subcommand(part);
         }
      }
      if( cmd ) {
         add_history(line);
         int r = cmd->run(line);
         printf("%d\n", r);
      } else {
         printf("Unknown command %s\n", line);
      }
      free(line);
   }
   printf("\n");
   return 0;
}
