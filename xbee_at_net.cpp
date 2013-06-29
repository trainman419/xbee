
#include "xbee_at_cmd.h"

command ** net_short() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "address", fake_cmd );
   *r++ = new command_child( "parent",  fake_cmd );
   *r++ = 0;
   return result;
};

command ** net_id() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "node",    fake_cmd );
   *r++ = new command_child( "cluster", fake_cmd );
   *r++ = 0;
   return result;
};

command ** net_max() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "unicast",   fake_cmd );
   *r++ = new command_child( "broadcast", fake_cmd );
   *r++ = 0;
   return result;
};

command ** net_pan() {
   command ** result = new command*[4];
   command ** r = result;
   *r++ = new command_child( "operating", fake_cmd );
   *r++ = new command_child( "long",      fake_cmd );
   *r++ = new command_child( "short",     fake_cmd );
   *r++ = 0;
   return result;
};

command ** net_node() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "timeout", fake_cmd );
   *r++ = new command_child( "options", fake_cmd );
   *r++ = 0;
   return result;
};

command ** net_join() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "notification", fake_cmd );
   *r++ = new command_child( "time",         fake_cmd );
   *r++ = 0;
   return result;
};

command ** net() {
   command ** result = new command*[11];
   command ** r = result;
   *r++ = new command_parent( "short",          net_short() );
   *r++ = new command_parent( "ID",             net_id()    );
   *r++ = new command_parent( "max-hops",       net_max()   );
   *r++ = new command_parent( "PAN-ID",         net_pan()   );
   *r++ = new command_parent( "node-discovery", net_node()  );
   *r++ = new command_parent( "join",           net_join()  );

   *r++ = new command_child( "max-payload", fake_cmd );
   *r++ = new command_child( "destination", fake_cmd );
   *r++ = new command_child( "children",    fake_cmd );
   *r++ = new command_child( "serial",      fake_cmd );
   *r++ = 0;
   return result;
};
