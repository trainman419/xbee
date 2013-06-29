
#include "xbee_at_cmd.h"

command ** rf_scan() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "channels", fake_cmd ),
   *r++ = new command_child( "duration", fake_cmd ),
   *r++ = 0;
   return result;
}

command ** rf_power() {
   command ** result = new command*[4];
   command ** r = result;
   *r++ = new command_child( "level", fake_cmd ),
   *r++ = new command_child( "mode",  fake_cmd ),
   *r++ = new command_child( "peak",  fake_cmd ),
   *r++ = 0;
   return result;
}

command ** rf() {
   command ** result = new command*[7];
   command ** r = result;
   *r++ = new command_parent( "scan",  rf_scan()  ),
   *r++ = new command_parent( "power", rf_power() ),

   *r++ = new command_child( "operating-channel",    fake_cmd ),
   *r++ = new command_child( "channel-verification", fake_cmd ),
   *r++ = new command_child( "zigbee-profile",       fake_cmd ),
   *r++ = new command_child( "RSSI",                 fake_cmd ),
   *r++ = 0;
   return result;
}

