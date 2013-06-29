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
#include "xbee_at_enc.h"
#include "xbee_at_net.h"
#include "xbee_at_rf.h"
#include "xbee_at_sleep.h"

#include <boost/foreach.hpp>

#include <stdio.h>

class fake_at_cmd : public at_cmd {
   public:
      fake_at_cmd() : at_cmd("") {}

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

class at_cmd_discover : public at_cmd_ro {
   public:
      at_cmd_discover() : at_cmd_ro("ND") {}

   protected:
      virtual int read(xbsh_state * state) {
         state->send_AT(cmd);
         api_frame * ret = state->read_AT();
         while( ret && ret->ok() && ret->get_data().size() > 0 ) {
            printf("Got %zd bytes\n", ret->get_data().size());
            ret = state->read_AT();
         }
         if( ret && ! ret->ok() ) {
            printf("Error: %s\n", ret->get_error().c_str());
            return 1;
         }
         printf("Discovery done\n");
         return 0;
      }
};

command ** toplevel() {
   command ** result = new command*[18];
   command ** r = result;
   *r++ = new command_parent( "diag",       diag()    );
   *r++ = new command_parent( "at",         at_c()    );
   *r++ = new command_parent( "reset",      reset_c() );
   *r++ = new command_parent( "io",         io()      );
   *r++ = new command_parent( "encryption", enc()     );
   *r++ = new command_parent( "net",        net()     );
   *r++ = new command_parent( "rf",         rf()      );
   *r++ = new command_parent( "serial",     serial_c());
   *r++ = new command_parent( "sleep",      sleep_c() );

   *r++ = new command_child( "discover-nodes", new at_cmd_discover() );
   *r++ = new command_child( "resolve-ni",       fake_cmd );
   *r++ = new command_child( "comission",        fake_cmd );
   *r++ = new command_child( "apply", new at_cmd_simple("AC") );
   *r++ = new command_child( "write", new at_cmd_simple("WR") );
   *r++ = new command_child( "defaults", new at_cmd_simple("RE") );
   *r++ = new command_child( "device-type",      fake_cmd );
   *r++ = new command_child( "debug",  new at_cmd_debug() );

   *r++ = 0;

   return result;
};

command * setup_commands() {
   return new command_parent( "<global>", toplevel() );
}
