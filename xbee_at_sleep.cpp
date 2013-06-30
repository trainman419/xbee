
#include "xbee_at_cmd.h"

command ** sleep_c() {
   command ** result = new command*[9];
   command ** r = result;
   *r++ = new command_child( "mode", new at_cmd_enum("SM", 4,
            0, "disabled",
            1, "pin-sleep",
            4, "cyclic",
            5, "cyclic-pin-wake"));
   *r++ = new command_child( "period-count", fake_cmd );
   *r++ = new command_child( "period-time",  fake_cmd );
   *r++ = new command_child( "timeout",      fake_cmd );
   *r++ = new command_child( "options",      fake_cmd );
   *r++ = new command_child( "wake-host",    fake_cmd );
   *r++ = new command_child( "now", new at_cmd_simple("SI") );
   *r++ = new command_child( "poll-rate",    fake_cmd );
   *r++ = 0;
   return result;
};
