/* xcsh2.cpp
 *
 * New XBee shell
 *
 * Author: Austin Hendrix
 */


#include <serial/serial.h>

#include <string>

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "xbee_api.h"
#include "prefix_map.h"

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics
typedef int (*command_f)(char * args);

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

class command {
   private:
      command_f cmd;
      std::string help;
      std::string name;

   public:
      command(std::string n, command_f c, std::string h) : name(n), cmd(c),
         help(h) {}

      int run(char * args) { return cmd(args); }
      std::string get_name() { return name; }
};

int fake_cmd(char * args) {
   printf("Fake command: %s\n", args);
}

prefix_map<command> commands;

char * xbsh_completion_function(const char * text, int state) {
   static std::list<std::string> completions;
   static std::list<std::string>::iterator itr;
   if( state == 0 ) {
      completions = commands.get_keys(text);
      itr = completions.begin();
   }
   char * res = 0;
   if( itr != completions.end() ) {
      res = (char*)malloc(itr->length()+1);
      memcpy(res, itr->c_str(), itr->length()+1);
      itr++;
   }
   return res;
}

int main(int argc, char ** argv) { 
   // initialize readline
   rl_completion_entry_function = xbsh_completion_function;
   using_history();

   // initialize command tree:
   command * foo = new command("foo", fake_cmd, "");
   command * bar = new command("bar", fake_cmd, "");
   command * baz = new command("baz", fake_cmd, "");
   commands.put(foo->get_name(), foo);
   commands.put(bar->get_name(), foo);
   commands.put(baz->get_name(), foo);

   // readline main loop
   char * line;
   while( line = readline("xbsh> ") ) {
      command * f = commands.get(line);
      if( f ) {
         add_history(line);
         int r = f->run(line);
         printf("%d\n", r);
      } else {
         printf("Unknown command %s\n", line);
      }
      free(line);
   }
   return 0;
}
