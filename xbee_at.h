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

command ** diag();
command ** at_c();
command ** reset_c();

#endif
