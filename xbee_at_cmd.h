/*
 * Command framework for xbsh
 *
 * Author: Austin Hendrix
 */

#ifndef XBEE_AT_CMD_H
#define XBEE_AT_CMD_H

#include <string>
#include <list>

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics
typedef int (*command_f)(const char * args);

class command {
   protected:
      std::string help;
      std::string name;

      command(std::string n, std::string h) : name(n), help(h) {}

   public:
      virtual int run(std::string args) { return 0; }
      virtual std::string get_name() { return name; }
      virtual std::string get_help() { return help; }

      virtual command * get_subcommand(std::string prefix) { 
         return NULL;
      }

      virtual std::list<std::string> get_completions(std::string prefix) {
         return std::list<std::string>();
      }
};

int fake_cmd(const char * args);

command * setup_commands();

#endif
