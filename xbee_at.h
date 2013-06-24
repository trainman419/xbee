// AT commands (mapping to AT commands):
//
//
//  diag FW-version (ATVR)
//  diag HW-version (ATHV)
//  diag associate-status (ATAI)
//
//  AT mode-timeout (ATCT)
//  AT end command mode (ATCN)
//  AT guard-time (ATGT)
//  AT command-character (ATCC)
//
//  reset soft (ATFR)
//  reset network (ATNR)
//  reset hard (toggle reset line)
//
//  discover-nodes (ATND)
//  device-type (ATDD)
//  apply (ATAC)
//  write (ATWR)
//  defaults (ATRE)
//  comission (ATCB)
//  resolve-NI (ATDN)
//
//  TODO: move to I/O command set
//  force IO sample (ATIS)
//  force sensor sample (AT1S)
//
//  TODO: shell buitins that don't correspond to AT commands:
//   show configuration
//   dump configuration to file
//   load config from file
//   change serial port parameters on-the-fly
//    - use case: update baud rate setting, update serial port to match
//    - serial rate unknown
//   hard reset via DTR line
//   firmware loading

#ifndef XBEE_AT_H
#define XBEE_AT_H

class at_cmd;

#include "xbsh2.h"
#include "xbee_at_cmd.h"
#include "prefix_map.h"

#include <string>

// get the AT command for a given string
//typedef std::string (*command_at)(std::string args);

class at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string arg) = 0; 

      virtual std::list<std::string> get_completions(std::string prefix) {
         return std::list<std::string>();
      }
};

class at_cmd_ro : public at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      virtual int read(xbsh_state * state) = 0;
};

class at_cmd_rw : public at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      virtual int read(xbsh_state * state) = 0;
      virtual int write(xbsh_state * state, std::string arg) = 0;
};

class at_cmd_wo : public at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      virtual int write(xbsh_state * state, std::string arg) = 0;
};

class at_cmd_debug : public at_cmd_rw {
   public:
      virtual std::list<std::string> get_completions(std::string prefix);

   protected:
      virtual int read(xbsh_state * state);
      virtual int write(xbsh_state * state, std::string arg);
};

class at_cmd_enum : public at_cmd_rw {
   private:
      std::string cmd;
      std::map<int, std::string> values;
      prefix_map<int> keys;

   public:
      at_cmd_enum(std::string c, int n, ...);

      virtual int read(xbsh_state * state);
      virtual int write(xbsh_state * state, std::string arg);
      virtual std::list<std::string> get_completions(std::string prefix);
};

class at_cmd_flags : public at_cmd_rw {
   // assumptions: all of the flag fields are continuous, starting at bit 0,
   // and all are two bytes in size
   private:
      std::string cmd;
      prefix_map<int> keys;
      std::map<int, std::string> values;

      std::list<std::string> split(std::string in);

   public:
      at_cmd_flags(std::string c, int n, ...);

      virtual int read(xbsh_state * state);
      virtual int write(xbsh_state * state, std::string arg);
      virtual std::list<std::string> get_completions(std::string prefix);
};

command ** diag();
command ** at_c();
command ** reset_c();

#endif
