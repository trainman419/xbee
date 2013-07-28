
#include "xbee_at_enc.h"

std::list<command*> enc() {
   std::list<command*> res;
   res.push_back(new command_child( "link-key",   fake_cmd));
   res.push_back(new command_child( "enable", new at_cmd_enum("EE", 2,
            0, "on", 1, "off")));
   res.push_back(new command_child( "options", new at_cmd_flags("EO", 2, 
            "unsecured-key", "trust-center")));
   res.push_back(new command_child( "key",        fake_cmd));

   return res;
}

