
#include "xbee_at_cmd.h"

command ** sleep_c() {
   command ** result = new command*[9];
   command ** r = result;
   *r++ = new command_child( "mode", new at_cmd_enum("SM", 4,
            0, "disabled",
            1, "pin-sleep",
            4, "cyclic",
            5, "cyclic-pin-wake"));
   *r++ = new command_child( "period-count", new at_cmd_scaled("SN", 2, 1,
            0xFFFF, 1, "", "Sleep periods"));
   *r++ = new command_child( "period-time", new at_cmd_scaled("SP", 2, 0x20,
            0xAF0, 10, "ms", "Sleep period"));
   *r++ = new command_child( "timeout", new at_cmd_scaled("ST", 2, 1, 0xFFFE,
            1, "ms", "Sleep timeout"));
   *r++ = new command_child( "options", new at_cmd_enum("SO", 4,
            0, "normal",
            2, "always-wake-ST", 
            4, "sleep-full-SN-SP",
            6, "extended"));
   *r++ = new command_child( "wake-host", new at_cmd_scaled("WH", 2, 0, 
            0xFFFF, 1, "ms", "Wake host timer")); // TODO: 0 for disabled
   *r++ = new command_child( "now", new at_cmd_simple("SI") );
   *r++ = new command_child( "poll-rate", new at_cmd_scaled("PO", 2,
            0, 0x3E8, 10, "ms", "Polling rate")); // TODO: 0 for default
   *r++ = 0;
   return result;
};
