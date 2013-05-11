
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

      virtual int run(char * args);
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

int command_child::run(char * args) {
   return cmd(args);
}

command * serial[] = {
   new command_parent( "api", NULL),

   new command_child( "baud",          fake_cmd),
   new command_child( "parity",        fake_cmd),
   new command_child( "stop",          fake_cmd),
   new command_child( "packetization", fake_cmd),
   new command_child( "DIO6",          fake_cmd),
   new command_child( "DIO7",          fake_cmd),
   0
};

command * enc[] = {
   new command_child( "link-key",   fake_cmd),
   new command_child( "enable",     fake_cmd),
   new command_child( "options",    fake_cmd),
   new command_child( "key",        fake_cmd),
   0
};

command * io_config[] = {
   new command_child( "pull-up", fake_cmd),
   new command_child( "PWM0",    fake_cmd),
   // DIO
   new command_child( "DIO0",    fake_cmd),
   new command_child( "DIO1",    fake_cmd),
   new command_child( "DIO2",    fake_cmd),
   new command_child( "DIO3",    fake_cmd),
   new command_child( "DIO4",    fake_cmd),
   new command_child( "DIO5",    fake_cmd),
   // DIO6-7 are serial
   new command_child( "DIO8",    fake_cmd),
   // DIO9-10 are serial
   new command_child( "DIO11",    fake_cmd),
   new command_child( "DIO12",    fake_cmd),
   new command_child( "DIO13",    fake_cmd),

   // A2D
   new command_child( "AD0",      fake_cmd),
   new command_child( "AD1",      fake_cmd),
   new command_child( "AD2",      fake_cmd),
   new command_child( "AD3",      fake_cmd),
   
   0
};

command * io_voltage[] = {
   new command_child( "supply",     fake_cmd),
   new command_child( "monitoring", fake_cmd),
   0
};

command * io[] = {
   new command_parent( "config",    io_config),
   new command_parent( "voltage",   io_voltage),

   new command_child( "sample-rate",      fake_cmd),
   new command_child( "change-detection", fake_cmd),
   new command_child( "led-blink-time",   fake_cmd),
   new command_child( "RSSI-PWM",         fake_cmd),
   new command_child( "temperature",      fake_cmd),
   0
};

command * toplevel[] = {
   new command_parent( "diag",       NULL ),
   new command_parent( "at",         NULL ),
   new command_parent( "reset",      NULL ),
   new command_parent( "discover",   NULL ),
   new command_parent( "io",         io ),
   new command_parent( "encryption", enc ),
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
