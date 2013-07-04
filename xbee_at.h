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
   protected:
      std::string cmd;
      at_cmd(std::string c) : cmd(c) {}

   public:
      virtual int run(xbsh_state * state, std::string arg) = 0; 

      virtual std::list<std::string> get_completions(std::string prefix) {
         return std::list<std::string>();
      }
};

class at_cmd_rw : public at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      at_cmd_rw(std::string c) : at_cmd(c) {}
      virtual int read(xbsh_state * state);
      virtual int write(xbsh_state * state, std::string arg);

      virtual int read_frame(xbsh_state * state, api_frame * ret) { return -1; }
      virtual std::vector<uint8_t> write_frame(xbsh_state * state, 
            std::string arg) { return std::vector<uint8_t>(); }
};

class at_cmd_ro : public at_cmd_rw {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      at_cmd_ro(std::string c) : at_cmd_rw(c) {}
};

class at_cmd_wo : public at_cmd_rw {
   public:
      virtual int run(xbsh_state * state, std::string);

   protected:
      at_cmd_wo(std::string c) : at_cmd_rw(c) {}
};

class at_cmd_debug : public at_cmd_rw {
   public:
      virtual std::list<std::string> get_completions(std::string prefix);
      at_cmd_debug() : at_cmd_rw("") {}

   protected:
      virtual int read(xbsh_state * state);
      virtual int write(xbsh_state * state, std::string arg);
};

class at_cmd_enum : public at_cmd_rw {
   private:
      std::map<int, std::string> values;
      prefix_map<int> keys;

   public:
      at_cmd_enum(std::string c, int n, ...);
      virtual std::list<std::string> get_completions(std::string prefix);

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret);
      virtual std::vector<uint8_t> write_frame(xbsh_state * state, 
            std::string arg);
};

class at_cmd_flags : public at_cmd_rw {
   // assumptions: all of the flag fields are continuous, starting at bit 0,
   // and all are two bytes in size
   private:
      prefix_map<int> keys;
      std::map<int, std::string> values;

      std::list<std::string> split(std::string in);

   public:
      at_cmd_flags(std::string c, int n, ...);
      virtual std::list<std::string> get_completions(std::string prefix);

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret);
      virtual std::vector<uint8_t> write_frame(xbsh_state * state,
            std::string arg);
};

class at_cmd_scaled : public at_cmd_rw {
   private:
      int bytes;
      int low;
      int high;
      double scale;
      std::string units;
      std::string flavor;

   public:
      at_cmd_scaled(std::string cmd, int bytes, int low, int high,
            double scale, std::string units, std::string flavor);
      virtual std::list<std::string> get_completions(std::string prefix);

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret);
      virtual std::vector<uint8_t> write_frame(xbsh_state * state,
            std::string arg);
};

class at_cmd_simple : public at_cmd {
   public:
      at_cmd_simple(std::string cmd) : at_cmd(cmd) {}
      virtual int run(xbsh_state * state, std::string arg);
};

class at_cmd_ro_hex : public at_cmd_ro {
   private:
      std::string flavor;
      int len;

   public:
      at_cmd_ro_hex(std::string at, std::string f, int l) : 
        at_cmd_ro(at), flavor(f), len(l) {}

      virtual int read(xbsh_state * state);
};

xbee_addr parse_addr(std::string addr);
std::string print_addr(xbee_addr addr);

command ** diag();
command ** at_c();
command ** reset_c();

#endif
