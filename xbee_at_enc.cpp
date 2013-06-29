
#include "xbee_at_cmd.h"

command ** enc() {
   command ** result = new command*[5];
   command ** r = result;
   *r++ = new command_child( "link-key",   fake_cmd);
   *r++ = new command_child( "enable",     fake_cmd);
   *r++ = new command_child( "options",    fake_cmd);
   *r++ = new command_child( "key",        fake_cmd);
   *r++ = 0;

   return result;
}

