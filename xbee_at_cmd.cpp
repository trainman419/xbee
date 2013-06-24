// how do we pass the serial port around?
//  variable arguments to run():
//    state class
//    command arguments
//  run() arguments passed all the way down the stack
//  run() does:
//   * argument parsing
//   * packet construction
//   * send over serial port
//   * wait for response(s)
//   * data reception
//   * response parsing
//   * response printing

#include "xbee_at.h"
#include "xbee_at_cmd.h"
#include "xbee_at_io.h"
#include "xbee_at_serial.h"
#include "prefix_map.h"

#include <boost/foreach.hpp>

#include <stdio.h>

/*
int fake_cmd(const char * args) {
   printf("Fake command: %s\n", args);
   return 0;
}
*/

class fake_at_cmd : public at_cmd {
   public:
      virtual int run(xbsh_state * state, std::string args) {
         printf("Fake command: %s\n", args.c_str());
         return 0;
      }
};

at_cmd * fake_cmd = new fake_at_cmd();

// types of child commands:
//  read-only
//  read/write
//  write-only
//  command (disassociate, comission, etc)
//
// types of values:
//  symbolic
//  bitmask
//  numeric scaled w/units
//  string (length limit)
//  fixed-size hex
//
// types of responses:
//  no response
//  single response
//  variable number of responses (null-terminated?)
//  asynchronous responses (handle as "no response")

command_parent::command_parent(std::string n, command ** sub) :
   command(n, "parent")
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

int command_parent::run(xbsh_state * state, std::string args) {
   std::list<std::string> sub = subcommands.get_keys(args);
   printf("\n");
   unsigned long maxlen = 0;
   BOOST_FOREACH( std::string s, sub ) {
      maxlen = std::max(maxlen, s.length());
   }
   BOOST_FOREACH( std::string s, sub ) {
      unsigned long l = maxlen - s.length();
      std::string pad(" ");
      for( unsigned long i = 0; i<l; i++ ) {
         pad += " ";
      }
      command * c = subcommands.get(s);
      if( c ) {
         std::string help = c->get_help();
         printf("   %s:%s%s\n", s.c_str(), pad.c_str(), help.c_str());
      } else {
         printf("Error: couldn't find subcommand %s\n", s.c_str());
      }
   }
   printf("\n");
   return 0;
}

int command_child::run(xbsh_state * state, std::string args) {
   return cmd->run(state, args.c_str());
}

std::list<std::string> command_child::get_completions(std::string prefix) {
   if(cmd) {
      return cmd->get_completions(prefix);
   } else {
      return std::list<std::string>();
   }
}

command * enc[] = {
   new command_child( "link-key",   fake_cmd),
   new command_child( "enable",     fake_cmd),
   new command_child( "options",    fake_cmd),
   new command_child( "key",        fake_cmd),
   0
};

command * net_short[] = {
   new command_child( "address", fake_cmd ),
   new command_child( "parent",  fake_cmd ),
   0
};

command * net_id[] = {
   new command_child( "node",    fake_cmd ),
   new command_child( "cluster", fake_cmd ),
   0
};

command * net_max[] = {
   new command_child( "unicast",   fake_cmd ),
   new command_child( "broadcast", fake_cmd ),
   0
};

command * net_pan[] = {
   new command_child( "operating", fake_cmd ),
   new command_child( "long",      fake_cmd ),
   new command_child( "short",     fake_cmd ),
   0
};

command * net_node[] = {
   new command_child( "timeout", fake_cmd ),
   new command_child( "options", fake_cmd ),
   0
};

command * net_join[] = {
   new command_child( "notification", fake_cmd ),
   new command_child( "time",         fake_cmd ),
   0
};

command * net[] = {
   new command_parent( "short",          net_short ),
   new command_parent( "ID",             net_id    ),
   new command_parent( "max-hops",       net_max   ),
   new command_parent( "PAN-ID",         net_pan   ),
   new command_parent( "node-discovery", net_node  ),
   new command_parent( "join",           net_join  ),

   new command_child( "max-payload", fake_cmd ),
   new command_child( "destination", fake_cmd ),
   new command_child( "children",    fake_cmd ),
   new command_child( "serial",      fake_cmd ),
   0
};

command * rf_scan[] = {
   new command_child( "channels", fake_cmd ),
   new command_child( "duration", fake_cmd ),
   0
};

command * rf_power[] = {
   new command_child( "level", fake_cmd ),
   new command_child( "mode",  fake_cmd ),
   new command_child( "peak",  fake_cmd ),
   0
};

command * rf[] = {
   new command_parent( "scan",  rf_scan  ),
   new command_parent( "power", rf_power ),

   new command_child( "operating-channel",    fake_cmd ),
   new command_child( "channel-verification", fake_cmd ),
   new command_child( "zigbee-profile",       fake_cmd ),
   new command_child( "RSSI",                 fake_cmd ),
   0
};

command * sleep_c[] = {
   new command_child( "mode",         fake_cmd ),
   new command_child( "period-count", fake_cmd ),
   new command_child( "period-time",  fake_cmd ),
   new command_child( "timeout",      fake_cmd ),
   new command_child( "options",      fake_cmd ),
   new command_child( "wake-host",    fake_cmd ),
   new command_child( "now",          fake_cmd ),
   new command_child( "poll-rate",    fake_cmd ),
   0
};

command * toplevel[] = {
   new command_parent( "diag",       diag()    ),
   new command_parent( "at",         at_c()    ),
   new command_parent( "reset",      reset_c() ),
   new command_parent( "io",         io()      ),
   new command_parent( "encryption", enc       ),
   new command_parent( "net",        net       ),
   new command_parent( "rf",         rf        ),
   new command_parent( "serial",     serial_c()),
   new command_parent( "sleep",      sleep_c   ),

   new command_child( "discover-nodes",   fake_cmd ),
   new command_child( "resolve-ni",       fake_cmd ),
   new command_child( "comission",        fake_cmd ),
   new command_child( "apply",            fake_cmd ),
   new command_child( "write",            fake_cmd ),
   new command_child( "defaults",         fake_cmd ),
   new command_child( "device-type",      fake_cmd ),
   new command_child( "debug",  new at_cmd_debug() ),

   0
};

command * setup_commands() {
   return new command_parent( "<global>", toplevel );
}
