
#include "xbee_at_cmd.h"

command ** enc() {
   command ** result = new command*[5];
   command ** r = result;
   *r++ = new command_child( "link-key",   fake_cmd);
   *r++ = new command_child( "enable", new at_cmd_enum("EE", 2,
            0, "on", 1, "off"));
   *r++ = new command_child( "options", new at_cmd_flags("EO", 2, 
            "unsecured-key", "trust-center"));
   *r++ = new command_child( "key",        fake_cmd);
   *r++ = 0;

   return result;
}

