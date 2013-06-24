
#include "xbee_at_cmd.h"

command ** serial_api() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "escape",  fake_cmd);
   *r++ = new command_child( "options", fake_cmd);
   *r++ = 0;

   return result;
}

command ** serial_c() {
   command ** result = new command*[8];
   command ** r = result;
   *r++ = new command_parent( "api", serial_api());

   *r++ = new command_child( "baud",                  fake_cmd);
   *r++ = new command_child( "parity",                fake_cmd);
   *r++ = new command_child( "stop",                  fake_cmd);
   *r++ = new command_child( "packetization-timeout", fake_cmd);
   *r++ = new command_child( "DIO6",                  fake_cmd);
   *r++ = new command_child( "DIO7",                  fake_cmd);
   *r++ = 0;

   return result;
}



