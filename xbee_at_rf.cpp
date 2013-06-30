
#include "xbee_at_cmd.h"

command ** rf_scan() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "channels", new at_cmd_flags("SC", 16,
            "0x0B", "0x0C", "0x0D", "0x0E", "0x0F", "0x10", "0x11", "0x12",
            "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1A"));
   *r++ = new command_child( "duration", fake_cmd );
   *r++ = 0;
   return result;
}

command ** rf_power() {
   command ** result = new command*[4];
   command ** r = result;
   *r++ = new command_child( "level", fake_cmd );
   *r++ = new command_child( "mode",  new at_cmd_enum("PM", 2,
            0, "normal",
            1, "boost"));
   *r++ = new command_child( "peak",  fake_cmd );
   *r++ = 0;
   return result;
}

command ** rf() {
   command ** result = new command*[7];
   command ** r = result;
   *r++ = new command_parent( "scan",  rf_scan()  );
   *r++ = new command_parent( "power", rf_power() );

   *r++ = new command_child( "operating-channel",    fake_cmd );
   *r++ = new command_child( "channel-verification", new at_cmd_enum("JV", 2,
            0, "disabled",
            1, "enabled"));
   *r++ = new command_child( "zigbee-profile", new at_cmd_enum("ZS", 3,
            0, "0", 1, "1", 2, "2") );
   *r++ = new command_child( "RSSI",                 fake_cmd );
   *r++ = 0;
   return result;
}

