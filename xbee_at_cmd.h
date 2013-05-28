/*
 * Command framework for xbsh
 *
 * Author: Austin Hendrix
 */

#ifndef XBEE_AT_CMD_H
#define XBEE_AT_CMD_H

class command;

#include "xbsh2.h"
#include "xbee_at.h"
#include "prefix_map.h"

#include <string>
#include <list>

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics

//typedef int (*command_f)(const char * args);

class command {
   protected:
      std::string help;
      std::string name;

      command(std::string n, std::string h) : name(n), help(h) {}

   public:
      virtual int run(xbsh_state * state, std::string args) { return 0; }
      virtual std::string get_name() { return name; }
      virtual std::string get_help() { return help; }

      virtual command * get_subcommand(std::string prefix) { 
         return NULL;
      }

      virtual std::list<std::string> get_completions(std::string prefix) {
         return std::list<std::string>();
      }
};

class command_parent : public command {
   private:
      prefix_map<command> subcommands;

   public:
      command_parent(std::string n, command ** sub);

      virtual command * get_subcommand(std::string prefix);
      virtual std::list<std::string> get_completions(std::string prefix);
      virtual int run(xbsh_state * state, std::string args);
};

class command_child : public command {
   private:
      at_cmd * cmd;

   public:
      command_child(std::string n, at_cmd * c) : command(n, "child"), cmd(c) {}

      virtual int run(xbsh_state * state, std::string args);
};

extern at_cmd * fake_cmd;

command * setup_commands();

#endif
