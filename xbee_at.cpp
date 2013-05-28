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
      std::string flavor;
      int len;

   public:
      at_cmd_ro_hex(std::string at, std::string f, int l) : 
        at_str(at), flavor(f), len(l) {}

      virtual int read(xbsh_state * state) {
         state->send_AT(at_str, 0, 0);
         api_frame * result = state->read_AT();
         if( result ) {
            std::vector<uint8_t> data = result->get_data();
            if(data.size() != len) {
               printf("Error: Got %zd bytes, expected %d\n", data.size(), len);
               return 1;
            }
            printf("%s: ", flavor.c_str());
            BOOST_FOREACH( uint8_t u, data ) {
              printf("%02X", u);
            }
            printf("\n");
         } else {
            printf("Failed to parse response packet\n");
            return 1;
         }
         return 0;
      }
};

class at_cmd_status : public at_cmd_ro {
   public:
      virtual int read(xbsh_state * state) {
         state->send_AT("AI", 0, 0);
         api_frame * result = state->read_AT();
         if( result ) {
            std::vector<uint8_t> data = result->get_data();
            if( data.size() != 1 ) {
               printf("Got %zd bytes, expected 1\n", data.size());
               return 1;
            }
            uint8_t status = data[0];
            switch(status) {
               case 0:
                  printf("Successfully joined/formed network\n");
                  break;
               case 0x21:
                  printf("Scan found no PANs\n");
                  break;
               case 0x22:
                  printf("Scan found no valid PANs based on current SC and ID settings\n");
                  break;
               case 0x23:
                  printf("Valid Coordinator or Routers found, but they are not allowing joining (NJ expired)\n");
                  break;
               case 0x24:
                  printf("No joinable beacons were found\n");
                  break;
               case 0x25:
                  printf("Unexpected state, node should not be attempting to join at this time\n");
                  break;
               case 0x27:
                  printf("Node Joining attempt failed (typically due to incompatible security settings)\n");
                  break;
               case 0x2A:
                  printf("Coordinator Start attempt failed\n");
                  break;
               case 0x2B:
                  printf("Checking for an existing coordinator\n");
                  break;
               case 0x2C:
                  printf("Attempt to leave the network failed\n");
                  break;
               case 0xAB:
                  printf("Attempted to join a device that did not respond.\n");
                  break;
               case 0xAC:
                  printf("Secure join error - network security key received unsecured\n");
                  break;
               case 0xAD:
                  printf("Secure join error - network security key not received\n");
                  break;
               case 0xAF:
                  printf("Secure join error - joining device does not have the right preconfigured link key\n");
                  break;
               case 0xFF:
                  printf("Scanning for a ZigBee network (routers and end devices)\n");
                  break;
               default:
                  printf("Unknown associate status %02X\n", status);
                  return 1;
            }
         }
         return 0;
      }
};

command ** diag() {
   command ** r = new command*[4];
   r[0] = new command_child( "fw-version",       new at_cmd_ro_hex("VR",
         "Firmware version", 2) );
   r[1] = new command_child( "hw-version",       new at_cmd_ro_hex("HV", 
         "Hardware version", 2) );
   // TODO: parse and pretty-print results
   r[2] = new command_child( "associate-status", new at_cmd_status() );
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
