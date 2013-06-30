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
#include <stdarg.h>

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

int at_cmd_rw::read(xbsh_state * state) {
   state->send_AT(cmd);
   api_frame * ret = state->read_AT();
   if( ret ) {
      if( ret->ok() ) {
         return read_frame(state, ret);
      } else {
         printf("Error: %s\n", ret->get_error().c_str());
         return 1;
      }
   } else {
      printf("Didn't get response for command AT%s\n", cmd.c_str());
      return 1;
   }
}

int at_cmd_rw::write(xbsh_state * state, std::string arg) {
   std::vector<uint8_t> data = write_frame(state, arg);
   if( data.size() == 0 ) return 3;
   state->send_AT(cmd, data);
   api_frame * ret = state->read_AT();
   if( ret ) {
      if( ret->ok() ) {
         printf("Success\n");
         return 0;
      } else {
         printf("Error: %s\n", ret->get_error().c_str());
         return 1;
      }
   } else {
      printf("Error: didn't get confirmation\n");
      return 2;
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

int at_cmd_ro_hex::read(xbsh_state * state) {
   state->send_AT(cmd);
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

class at_cmd_status : public at_cmd_ro {
   public:
      at_cmd_status() : at_cmd_ro("AI") {}

      virtual int read_frame(xbsh_state * state, api_frame * result) {
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
         return 0;
      }
};

std::list<std::string> at_cmd_debug::get_completions(std::string prefix) {
   std::list<std::string> res;
   if( prefix == "" || prefix == "o" ) {
      res.push_back("on");
      res.push_back("off");
   } else if( prefix == "on" ) {
      res.push_back("on");
   } else if( prefix == "of" || prefix == "off" ) {
      res.push_back("off");
   }
   return res;
}

int at_cmd_debug::read(xbsh_state * state) {
   if( state->debug ) {
      printf("debugging on\n");
   } else {
      printf("debugging off\n");
   }
   return 0;
}

int at_cmd_debug::write(xbsh_state * state, std::string arg) {
   if( arg == "on" ) {
      state->debug = 1;
      printf("debuging on\n");
      return 0;
   } else if( arg == "off" ) {
      state->debug = 0;
      printf("debugging off\n");
      return 0;
   } else {
      printf("Unreconized debugging option: %s\n", arg.c_str());
      return 1;
   }
}

at_cmd_enum::at_cmd_enum(std::string c, int n, ...) : at_cmd_rw(c) {
   va_list vl;
   int idx;
   char * val;
   va_start(vl, n);
   for( int i=0; i<n; i++ ) {
      idx = va_arg(vl, int);
      val = va_arg(vl, char*);
      values[idx] = val;
      int * n = new int;
      *n = idx;
      keys.put(val, n);
   }
   va_end(vl);
}

int at_cmd_enum::read_frame(xbsh_state * state, api_frame * ret) {
   std::vector<uint8_t> data = ret->get_data();
   if( data.size() == 1 ) {
      if( values.count(data[0]) == 1 ) {
         printf("%s\n", values[data[0]].c_str());
         return 0;
      } else {
         printf("Got Unknown result %d\n", data[0]);
         return 1;
      }
   } else {
      printf("Expected 1 byte; got %zd bytes\n", data.size());
      return 1;
   }
}

std::vector<uint8_t> at_cmd_enum::write_frame(xbsh_state * state, std::string arg) {
   int * n = keys.get(arg);
   std::vector<uint8_t> ret;
   if( n ) {
      ret.push_back(*n);
   } else {
      printf("Bad value %s\n", arg.c_str());
   }
   return ret;
}

std::list<std::string> at_cmd_enum::get_completions(std::string prefix) {
   return keys.get_keys(prefix);
}

// split a string on commas
std::list<std::string> at_cmd_flags::split(std::string in) {
   int start = 0;
   std::list<std::string> res;
   int i;
   for( i=0; i<in.length(); i++ ) {
      if( in[i] == ',' ) {
         res.push_back(in.substr(start, i-start));
         start = i+1;
      }
   }
   if( start <= i ) {
      res.push_back(in.substr(start, i-start));
   }
   return res;
}

at_cmd_flags::at_cmd_flags(std::string c, int n, ...) : at_cmd_rw(c) {
   va_list vl;
   int idx = 1;
   char * val;
   va_start(vl, n);
   for( int i=0; i<n; i++ ) {
      val = va_arg(vl, char*);
      values[idx] = val;
      int * n = new int;
      *n = idx;
      keys.put(val, n);
      idx <<= 1;
   }
   va_end(vl);
}

int at_cmd_flags::read_frame(xbsh_state * state, api_frame * ret) {
   std::vector<uint8_t> data = ret->get_data();
   if( data.size() == 2 ) {
      int flags = data[0] << 8 | data[1];
      std::list<std::string> res;
      for( int i=1; i<0xFFFF; i <<= 1 ) {
         if( i & flags ) {
            res.push_back(values[i]);
         }
      }
      res.sort();
      std::string r;
      if( res.size() > 0 ) {
         r = res.front();
         res.pop_front();
      }
      BOOST_FOREACH(std::string p, res) {
         r += "," + p;
      }
      printf("%s\n", r.c_str());
      return 0;
   } else {
      printf("Expected 2 bytes, got %zd\n", data.size());
      return 3;
   }
}

std::vector<uint8_t> at_cmd_flags::write_frame(xbsh_state * state,
      std::string arg) {
   std::list<std::string> parts = split(arg);
   int flags = 0;
   int err = 0;
   BOOST_FOREACH(std::string part, parts) {
      int * n = keys.get(part);
      if( n ) {
         flags |= *n;
      } else {
         printf("Unknown flag: %s\n", part.c_str());
         err = 1;
      }
   }
   std::vector<uint8_t> ret;
   if( err ) {
      return ret;
   }
   char data[2];
   ret.push_back(flags >> 8);
   ret.push_back(flags);
   return ret;
}

std::list<std::string> at_cmd_flags::get_completions(std::string prefix) {
   // split on commas; feed last token into get_keys
   std::list<std::string> parts = split(prefix);
   std::string last;
   if( parts.size() > 0 ) {
      last = parts.back();
      parts.pop_back();
   }
   std::list<std::string> completions = keys.get_keys(last);
   std::string p;
   BOOST_FOREACH(std::string part, parts) {
      p += part + ",";
      for( std::list<std::string>::iterator itr = completions.begin();
            itr != completions.end(); ++itr ) {
         while( *itr == part ) {
            itr = completions.erase(itr);
         }
      }
   }
   std::list<std::string> res;
   BOOST_FOREACH(std::string comp, completions) {
      res.push_back(p + comp);
   }
   return res;
}

int at_cmd_simple::run(xbsh_state * state, std::string arg) {
   if( arg.length() > 0 ) {
      printf("This command does not take arguments\n");
      return 1;
   }
   state->send_AT(cmd);
   api_frame * ret = state->read_AT();
   if( ret ) {
      if( ret->ok() ) {
         printf("Success\n");
         return 0;
      } else {
         printf("Error: %s\n", ret->get_error().c_str());
         return 3;
      }
   } else {
      printf("Didn't get a response\n");
      return 2;
   }
}

class at_cmd_fw : public at_cmd_ro {
   public:
      at_cmd_fw() : at_cmd_ro("VR") {};

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret) {
         std::vector<uint8_t> data = ret->get_data();
         if( data.size() == 2 ) {
            printf("Firmware version: %02X%02X\n", data[0], data[1]);
            std::string type;
            switch(data[0]) {
               case 0x20:
                  type = "AT Coordinator";
                  break;
               case 0x21:
                  type = "API Coordinator";
                  break;
               case 0x22:
                  type = "AT Router";
                  break;
               case 0x23:
                  type = "API Router";
                  break;
               case 0x28:
                  type = "AT End Device";
                  break;
               case 0x29:
                  type = "API End Device";
                  break;
               default:
                  type = "Unknown";
                  break;
            }
            printf("%s\n", type.c_str());
            return 0;
         } else {
            printf("Got %zd bytes; expected 2\n", data.size());
            return 1;
         }
      }
};

command ** diag() {
   command ** r = new command*[4];
   r[0] = new command_child( "fw-version", new at_cmd_fw());
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
