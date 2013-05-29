
#include "xbee_at_cmd.h"

#include "prefix_map.h"

#include <map>
#include <string>

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
            printf("Didn't get response for command AT%s\n", cmd.c_str());
            return 1;
         }
      }

      virtual int write(xbsh_state * state, std::string arg) {
         int * n = keys.get(arg);
         if( n ) {
            printf("Send value %d\n", *n);
            printf("Unimplemented\n");
            return 2;
         } else {
            printf("Bad value %s\n", arg.c_str());
            return 1;
         }
      }
};

command ** io_config() {
   command ** result = new command*[17];
   command ** r = result;
   *r++ = new command_child( "pull-up", fake_cmd);
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
   *r++ = new command_child( "DIO00",    dio0);
   *r++ = new command_child( "DIO01",    dio1);
   *r++ = new command_child( "DIO02",    fake_cmd);
   *r++ = new command_child( "DIO03",    fake_cmd);
   *r++ = new command_child( "DIO04",    fake_cmd);
   *r++ = new command_child( "DIO05",    fake_cmd);
   // DIO6-7 are serial
   *r++ = new command_child( "DIO08",    fake_cmd);
   // DIO9-10 are serial
   *r++ = new command_child( "DIO11",    fake_cmd);
   *r++ = new command_child( "DIO12",    fake_cmd);
   *r++ = new command_child( "DIO13",    fake_cmd);

   // A2D
   *r++ = new command_child( "AD0",      dio0);
   *r++ = new command_child( "AD1",      dio1);
   *r++ = new command_child( "AD2",      fake_cmd);
   *r++ = new command_child( "AD3",      fake_cmd);
   
   *r++ = 0;
   return result;
}

command ** io_voltage() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "supply",     fake_cmd);
   *r++ = new command_child( "monitoring", fake_cmd);
   *r++ = 0;
   return result;
};

command ** io() {
   command ** result = new command*[8];
   command ** r = result;
   *r++ = new command_parent( "config",    io_config());
   *r++ = new command_parent( "voltage",   io_voltage());

   *r++ = new command_child( "sample-rate",      fake_cmd);
   *r++ = new command_child( "change-detection", fake_cmd);
   *r++ = new command_child( "led-blink-time",   fake_cmd);
   *r++ = new command_child( "RSSI-PWM",         fake_cmd);
   *r++ = new command_child( "temperature",      fake_cmd);
   *r++ = 0;
   return result;
}
