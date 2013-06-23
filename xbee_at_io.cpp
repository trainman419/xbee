
#include "xbee_at_cmd.h"

#include "prefix_map.h"

#include <map>
#include <string>

#include <boost/foreach.hpp>

#include <stdarg.h>

class at_cmd_enum : public at_cmd_rw {
   private:
      std::string cmd;
      std::map<int, std::string> values;
      prefix_map<int> keys;

   public:
      at_cmd_enum(std::string c, int n, ...) : cmd(c) {
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

      virtual int read(xbsh_state * state) {
         state->send_AT(cmd, 0, 0);
         api_frame * ret = state->read_AT();
         if( ret ) {
            if( ret->get_status() == 0 ) {
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
            } else {
               std::string err;
               switch(ret->get_status()) {
                  case 0:
                     err = "No Error??";
                     break;
                  case 1:
                     err = "ERROR";
                     break;
                  case 2:
                     err = "Invalid Command";
                     break;
                  case 3:
                     err = "Invalid Parameter";
                     break;
                  case 4:
                     err = "Tx Failure";
                     break;
                  default:
                     err = "Unknown error";
                     break;
               }
               printf("%s\n", err.c_str());
               return 1;
            }
         } else {
            printf("Didn't get response for command AT%s\n", cmd.c_str());
            return 1;
         }
      }

      virtual int write(xbsh_state * state, std::string arg) {
         int * n = keys.get(arg);
         if( n ) {
            char val[1];
            val[0] = *n;
            state->send_AT(cmd, val, 1);
            api_frame * ret = state->read_AT();
            if( ret ) {
               if( ret->get_status() == 0 ) {
                  printf("Success: %s\n", values[*n].c_str());
                  return 0;
               } else {
                  printf("Error: %d\n", ret->get_status());
                  return 2;
               }
            } else {
               printf("Error: didn't get confirmation\n");
               return 2;
            }
         } else {
            printf("Bad value %s\n", arg.c_str());
            return 1;
         }
      }

      virtual std::list<std::string> get_completions(std::string prefix) {
         return keys.get_keys(prefix);
      }
};

class at_cmd_flags : public at_cmd_rw {
   // assumptions: all of the flag fields are continuous, starting at bit 0,
   // and all are two bytes in size
   private:
      std::string cmd;
      prefix_map<int> keys;
      std::map<int, std::string> values;

      // split a string on commas
      std::list<std::string> split(std::string in) {
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

   public:
      at_cmd_flags(std::string c, int n, ...) : cmd(c) {
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

      virtual int read(xbsh_state * state) {
         state->send_AT(cmd, 0, 0);
         api_frame * ret = state->read_AT();
         if( ret ) {
            std::string err;
            switch( ret->get_status() ) {
               case 0:
                  {
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
                     } else {
                        printf("Expected 2 bytes, got %zd\n", data.size());
                        return 3;
                     }
                  }
                  break;
               case 1:
                  err = "ERROR";
                  break;
               case 2:
                  err = "Invalid Command";
                  break;
               case 3:
                  err = "Invalid Parameter";
                  break;
               case 4:
                  err = "Tx Failure";
                  break;
               default:
                  err = "Unknown error";
                  break;
            }
            if( err.length() > 0 ) {
               printf("%s\n", err.c_str());
               return 2;
            } else {
               return 0;
            }
         } else {
            printf("Didn't get a response for command AT%s\n", cmd.c_str());
            return 1;
         }
      }

      virtual int write(xbsh_state * state, std::string arg) {
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
         if( err ) {
            return 1;
         }
         char data[2];
         data[0] = flags >> 8;
         data[1] = flags;
         state->send_AT(cmd, data, 2);
         api_frame * ret = state->read_AT();
         if( ret ) {
            // TODO: better error checking
            if( ret->get_status() == 0 ) {
               printf("Success\n");
               return 0;
            } else {
               printf("Error\n");
               return 2;
            }
         } else {
            printf("Didn't get a response\n");
            return 1;
         }
      }

      virtual std::list<std::string> get_completions(std::string prefix) {
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
};

command ** io_voltage() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "supply",     fake_cmd);
   *r++ = new command_child( "monitoring", fake_cmd);
   *r++ = 0;
   return result;
};

command ** io() {
   command ** result = new command*[23];
   command ** r = result;
   *r++ = new command_parent( "voltage",   io_voltage());

   *r++ = new command_child( "pull-up", new at_cmd_flags("PR",14,
            "D4", "D3", "D2", "D1", "D0", "D6", "D8", "DIN", "D5",
            "D9", "D12", "D10", "D11", "D7"));
   *r++ = new command_child( "PWM0",    fake_cmd);

   // DIO
   at_cmd * dio0 = new at_cmd_enum("D0", 5, 
         1, "comissioning",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio1 = new at_cmd_enum("D1", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio2 = new at_cmd_enum("D2", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio3 = new at_cmd_enum("D3", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");

   // Digital commands
   *r++ = new command_child( "D0",    dio0);
   *r++ = new command_child( "D1",    dio1);
   *r++ = new command_child( "D2",    dio2);
   *r++ = new command_child( "D3",    dio3);

   *r++ = new command_child( "D4",    new at_cmd_enum("D4", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   *r++ = new command_child( "D5",    new at_cmd_enum("D5", 5,
            0, "disabled",
            1, "associate",
            3, "digital-in",
            4, "off",
            5, "on"));

   // DIO6-7 are serial
   *r++ = new command_child( "D8",    new at_cmd_enum("D8", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   // DIO9-10 are serial
   *r++ = new command_child( "D11",    new at_cmd_enum("P1", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   *r++ = new command_child( "D12",    new at_cmd_enum("P2", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   /*
   *r++ = new command_child( "D13",    new at_cmd_enum("P3", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
            */

   // A2D
   *r++ = new command_child( "AD0",      dio0);
   *r++ = new command_child( "AD1",      dio1);
   *r++ = new command_child( "AD2",      dio2);
   *r++ = new command_child( "AD3",      dio3);

   // other configurataion options
   *r++ = new command_child( "sample-rate",      fake_cmd);
   *r++ = new command_child( "change-detection", fake_cmd);
   *r++ = new command_child( "led-blink-time",   fake_cmd);
   *r++ = new command_child( "RSSI-PWM",         fake_cmd);
   *r++ = new command_child( "temperature",      fake_cmd);
   *r++ = 0;
   return result;
}
