
#include "xbee_at_cmd.h"

command ** sleep_c() {
   command ** result = new command*[9];
   command ** r = result;
   *r++ = new command_child( "mode",         fake_cmd );
   *r++ = new command_child( "period-count", fake_cmd );
   *r++ = new command_child( "period-time",  fake_cmd );
   *r++ = new command_child( "timeout",      fake_cmd );
   *r++ = new command_child( "options",      fake_cmd );
   *r++ = new command_child( "wake-host",    fake_cmd );
   *r++ = new command_child( "now",          fake_cmd );
   *r++ = new command_child( "poll-rate",    fake_cmd );
   *r++ = 0;
   return result;
};
