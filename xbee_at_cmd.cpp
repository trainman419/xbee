
#include "xbee_at_cmd.h"

#include <stdio.h>

int fake_cmd(char * args) {
   printf("Fake command: %s\n", args);
   return 0;
}

class command_parent : public command {
   public:
      command_parent(std::string n, command * sub);
};

command_parent::command_parent(std::string n, command * sub) :
   command(n, "")
{

}

command * toplevel[] = {
   new command_parent( "diag",       NULL ),
   new command_parent( "at",         NULL ),
   new command_parent( "apply",      NULL ),
   new command_parent( "write",      NULL ),
   new command_parent( "defaults",   NULL ),
   new command_parent( "reset",      NULL ),
   new command_parent( "comission",  NULL ),
   new command_parent( "discover",   NULL ),
   new command_parent( "resolve",    NULL ),
   new command_parent( "io",         NULL ),
   new command_parent( "encryption", NULL ),
   new command_parent( "net",        NULL ),
   new command_parent( "rf",         NULL ),
   new command_parent( "serial",     NULL ),
   new command_parent( "sleep",      NULL )
};

void setup_commands(prefix_map<command> * map) {
   for( int i=0; i<sizeof(toplevel)/sizeof(command*); i++ ) {
      map->put(toplevel[i]->get_name(), toplevel[i]);
   }
}
