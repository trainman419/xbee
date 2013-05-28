/*
 * xbee_at.cpp
 *
 * xbee AT base classes
 *
 * Author: Austin Hendrix
 */

#include "xbee_at.h"

#include <boost/foreach.hpp>
#include <stdint.h>

int at_cmd_ro::run(xbsh_state * state, std::string arg) {
   if( arg.length() > 0 ) {
      printf("Error: this property is read-only\n");
      return 1;
   } else {
      return read(state);
   }
}

int at_cmd_rw::run(xbsh_state * state, std::string arg) {
   if( arg.length() > 0 ) {
      return write(state, arg);
   } else {
      return read(state);
   }
}

int at_cmd_wo::run(xbsh_state * state, std::string arg) {
   if( arg.length() > 0 ) {
      return write(state, arg);
   } else {
      printf("Error: this property is write-only and requires an argument\n");
      return 1;
   }
}

class at_cmd_ro_hex : public at_cmd_ro {
   private:
      std::string at_str;
      int len;

   public:
      at_cmd_ro_hex(std::string at, int l) : at_str(at), len(l) {}

      virtual int read(xbsh_state * state) {
         state->send_AT(at_str, 0, 0);
         api_frame * result = state->read_AT();
         if( result ) {
            std::vector<uint8_t> data = result->get_data();
            BOOST_FOREACH( uint8_t u, data ) {
              printf("%02X", u);
            }
            printf("\n");
         } else {
            printf("Failed to parse response packet\n");
         }
         return 0;
      }
};

command ** diag() {
   command ** r = new command*[4];
   r[0] = new command_child( "fw-version",       new at_cmd_ro_hex("VR", 2) );
   r[1] = new command_child( "hw-version",       new at_cmd_ro_hex("HV", 2) );
   // TODO: parse and pretty-print results
   r[2] = new command_child( "associate-status", new at_cmd_ro_hex("AI", 1) );
   r[3] = 0;
   return r;
}

command ** at_c() {
   command ** r = new command*[4];
   r[0] = new command_child( "mode-timeout",      fake_cmd );
   r[1] = new command_child( "guard-time",        fake_cmd );
   r[2] = new command_child( "command-character", fake_cmd );
   r[3] = 0;
   return r;
}

command ** reset_c() {
   command ** r = new command*[4];
   r[0] = new command_child( "network", fake_cmd );
   r[1] = new command_child( "soft",    fake_cmd );
   r[2] = new command_child( "hard",    fake_cmd );
   r[3] = 0;
   return r;
}
