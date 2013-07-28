
#include "xbee_at_sleep.h"

std::list<command*> sleep_c() {
   std::list<command*> res;
   res.push_back(new command_child( "mode", new at_cmd_enum("SM", 4,
               0, "disabled",
               1, "pin-sleep",
               4, "cyclic",
               5, "cyclic-pin-wake")));
   res.push_back(new command_child( "period-count", new at_cmd_scaled("SN", 2,
               1, 0xFFFF, 1, "", "Sleep periods")));
   res.push_back(new command_child( "period-time", new at_cmd_scaled("SP", 2,
               0x20, 0xAF0, 10, "ms", "Sleep period")));
   res.push_back(new command_child( "timeout", new at_cmd_scaled("ST", 2, 1,
               0xFFFE, 1, "ms", "Sleep timeout")));
   res.push_back(new command_child( "options", new at_cmd_enum("SO", 4,
               0, "normal",
               2, "always-wake-ST", 
               4, "sleep-full-SN-SP",
               6, "extended")));
   res.push_back(new command_child( "wake-host", new at_cmd_scaled("WH", 2, 0, 
               0xFFFF, 1, "ms", "Wake host timer"))); // TODO: 0 for disabled
   res.push_back(new command_child( "now", new at_cmd_simple("SI") ));
   res.push_back(new command_child( "poll-rate", new at_cmd_scaled("PO", 2,
               0, 0x3E8, 10, "ms", "Polling rate"))); // TODO: 0 for default
   return res;
};
