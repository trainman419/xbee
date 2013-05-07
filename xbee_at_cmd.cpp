
#include "xbee_at_cmd.h"
#include "prefix_map.h"

#include <stdio.h>

int fake_cmd(char * args) {
   printf("Fake command: %s\n", args);
   return 0;
}

class command_parent : public command {
   private:
      prefix_map<command> subcommands;

   public:
      command_parent(std::string n, command ** sub);

      virtual command * get_subcommand(std::string prefix);
      virtual std::list<std::string> get_completions(std::string prefix);
};

class command_child : public command {
   private:
      command_f cmd;

   public:
      command_child(std::string n, command_f c) : command(n, ""), cmd(c) {}
};

command_parent::command_parent(std::string n, command ** sub) :
   command(n, "")
{
   if( sub ) {
      for( int i = 0; sub[i]; i++ ) {
         subcommands.put(sub[i]->get_name(), sub[i]);
      }
   }
}

command * command_parent::get_subcommand(std::string prefix) {
   return subcommands.get(prefix);
}

std::list<std::string> command_parent::get_completions(std::string prefix) {
   return subcommands.get_keys(prefix);
}

command * serial[] = {
   new command_parent( "api", NULL),

   new command_child( "baud", fake_cmd),
   new command_child( "parity", fake_cmd),
   new command_child( "stop", fake_cmd),
   new command_child( "packetization", fake_cmd),
   new command_child( "DIO6", fake_cmd),
   new command_child( "DIO7", fake_cmd),
   0
};

command * toplevel[] = {
   new command_parent( "diag",       NULL ),
   new command_parent( "at",         NULL ),
   new command_parent( "reset",      NULL ),
   new command_parent( "discover",   NULL ),
   new command_parent( "io",         NULL ),
   new command_parent( "encryption", NULL ),
   new command_parent( "net",        NULL ),
   new command_parent( "rf",         NULL ),
   new command_parent( "serial",     serial ),
   new command_parent( "sleep",      NULL ),

   new command_child( "resolve",    fake_cmd ),
   new command_child( "comission",  fake_cmd ),
   new command_child( "apply",      fake_cmd ),
   new command_child( "write",      fake_cmd ),
   new command_child( "defaults",   fake_cmd ),

   0
};

command * setup_commands() {
   return new command_parent( "<global>", toplevel );
}
