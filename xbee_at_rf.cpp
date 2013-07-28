
#include "xbee_at_rf.h"

std::list<command*> rf_scan() {
   std::list<command*> res;
   res.push_back(new command_child( "channels", new at_cmd_flags("SC", 16,
            "0x0B", "0x0C", "0x0D", "0x0E", "0x0F", "0x10", "0x11", "0x12",
            "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1A")));
   res.push_back(new command_child( "duration", new at_cmd_scaled("SD", 1, 0,
            7, 1, "", "Scan duration (channels * (2 ^ duration) * 15.36ms)")));
   return res;
}

std::list<command*> rf_power() {
   std::list<command*> res;
   res.push_back(new command_child( "level", fake_cmd ));
   res.push_back(new command_child( "mode", new at_cmd_enum("PM", 2,
            0, "normal", 1, "boost")));
   res.push_back(new command_child( "peak", new at_cmd_scaled_ro("PP", 1, 1.0,
               "dBm", "Peak power")));
   return res;
}

std::list<command*> rf() {
   std::list<command*> res;
   res.push_back(new command_parent( "scan", rf_scan() ));
   res.push_back(new command_parent( "power", rf_power() ));

   res.push_back(new command_child( "operating-channel", new at_cmd_scaled_ro(
               "CH", 1, 1, "", "Operating Channel")));
   res.push_back(new command_child( "channel-verification", new at_cmd_enum(
            "JV", 2, 0, "disabled", 1, "enabled")));
   res.push_back(new command_child( "zigbee-profile", new at_cmd_enum("ZS", 3,
            0, "0", 1, "1", 2, "2") ));
   res.push_back(new command_child( "RSSI", new at_cmd_scaled_ro("DB", 1, -1,
               "dBm", "Received signal strength")));
   return res;
}

