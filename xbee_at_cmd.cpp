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
//

#include "xbee_at.h"
#include "xbee_at_cmd.h"
#include "xbee_at_io.h"
#include "xbee_at_serial.h"
#include "xbee_at_enc.h"
#include "xbee_at_net.h"
#include "xbee_at_rf.h"
#include "xbee_at_sleep.h"

#include <boost/foreach.hpp>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>

class fake_at_cmd : public at_cmd {
   public:
      fake_at_cmd() : at_cmd("") {}

      virtual int run(xbsh_state * state, std::string args) {
         printf("Fake command: %s\n", args.c_str());
         return 0;
      }
};

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

command_parent::command_parent(std::string n, std::list<command*> cmds) :
   command(n, "parent"), subcommands()
{
   BOOST_FOREACH(command * c, cmds) {
      subcommands.put(c->get_name(), c);
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
            std::vector<uint8_t> data = ret->get_data();
            printf("Got %zd bytes\n", data.size());
            int i=0;
            uint16_t net_addr = data[i++];
            net_addr <<= 8;
            net_addr |= data[i++];;
            uint64_t serial = 0;
            for( int j=0; j<8; j++ ) {
              serial <<= 8;
              serial |= data[i++];
            }
            std::string ni;
            for( ; data[i]; i++ ) {
              ni.push_back(data[i]);
            }
            i++; // eat trailing null
            uint16_t parent = data[i++];
            parent <<= 8;
            parent |= data[i++];
            std::string type;
            switch(data[i++]) {
              case 0:
                type = "Coordinator";
                break;
              case 1:
                type = "Router";
                break;
              case 2:
                type = "End Device";
                break;
              default:
                type = "Unknown";
                break;
            }
            uint8_t status = data[i++]; // ?
            uint16_t profile = data[i++];
            profile <<= 8;
            profile |= data[i++];
            uint16_t manufacturer = data[i++];
            manufacturer <<= 8;
            manufacturer |= data[i++];
            printf("Net Address:  %04X\n", net_addr);
            printf("Serial:       %016" PRIX64 "\n", serial);
            printf("Node ID:      %s\n", ni.c_str());
            printf("Parent:       %04X\n", parent);
            printf("Type:         %s\n", type.c_str());
            printf("Status:       %d\n", status);
            printf("Profile:      %d\n", profile);
            printf("Manufacturer: %d\n", manufacturer);

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

class at_cmd_remote : public at_cmd_rw {
   public:
      at_cmd_remote() : at_cmd_rw("") {}

   protected:
      virtual int read(xbsh_state * state) {
         BOOST_FOREACH(xbee_addr addr, state->get_remotes()) {
            printf("%s\n", print_addr(addr).c_str());
         }
         return 0;
      }

      virtual int write(xbsh_state * state, std::string arg) {
         if( arg == "pop" ) {
            state->pop_remote();
         } else {
            try {
               state->push_remote(parse_addr(arg));
            } catch( std::exception &e ) {
               printf("%s\n", e.what());
               return 1;
            }
         }
         return 0;
      }
};

class at_cmd_resolve : public at_cmd {
   public:
      at_cmd_resolve() : at_cmd("DN") {}
      virtual ~at_cmd_resolve() {}

   protected:
      virtual int run(xbsh_state * state, std::string arg) {
         if( arg.length() < 1 ) {
            printf("Error: please supply a node identifier\n");
            return 1;
         }

         if( arg.length() > 20 ) {
            printf("Error: node identifier too long\n");
            return 1;
         }

         std::vector<uint8_t> data(arg.begin(), arg.end());
         state->send_AT(cmd, data);
         api_frame * ret = state->read_AT();
         if( ret->ok() ) {
            std::vector<uint8_t> ret_data = ret->get_data();
            if( ret_data.size() == 10 ) {
               xbee_net net;
               net.c_net[1] = ret_data[0];
               net.c_net[0] = ret_data[1];

               xbee_addr addr;
               addr.c_addr[0] = ret_data[2];
               addr.c_addr[1] = ret_data[3];
               addr.c_addr[2] = ret_data[4];
               addr.c_addr[3] = ret_data[5];

               addr.c_addr[4] = ret_data[6];
               addr.c_addr[5] = ret_data[7];
               addr.c_addr[6] = ret_data[8];
               addr.c_addr[7] = ret_data[9];
               printf("Network Address:  %02X%02X\n", net.c_net[1],
                     net.c_net[0]);
               printf("Extended Address: %s\n", print_addr(addr).c_str());
            } else {
               printf("Expected 10 bytes, got %zd\n", ret_data.size());
               return 2;
            }
         } else {
            printf("Error: %s\n", ret->get_error().c_str());
            return 1;
         }
         return 0;
      }
};

std::list<command*> toplevel() {
   std::list<command*> res;
   res.push_back(new command_parent( "diagnostic", diag()    ));
   res.push_back(new command_parent( "at",         at_c()    ));
   res.push_back(new command_parent( "reset",      reset_c() ));
   res.push_back(new command_parent( "io",         io()      ));
   res.push_back(new command_parent( "encryption", enc()     ));
   res.push_back(new command_parent( "net",        net()     ));
   res.push_back(new command_parent( "rf",         rf()      ));
   res.push_back(new command_parent( "serial",     serial_c()));
   res.push_back(new command_parent( "sleep",      sleep_c() ));

   res.push_back(new command_child( "discover-nodes", new at_cmd_discover() ));
   res.push_back(new command_child( "resolve-ni", new at_cmd_resolve()));
   res.push_back(new command_child( "comission", new at_cmd_option("CB",1,2)));
   res.push_back(new command_child( "apply", new at_cmd_simple("AC") ));
   res.push_back(new command_child( "write", new at_cmd_simple("WR") ));
   res.push_back(new command_child( "defaults", new at_cmd_simple("RE") ));
   res.push_back(new command_child( "device-type", new at_cmd_hex("DD", 4,
               "Device Type") ));
   res.push_back(new command_child( "debug",  new at_cmd_debug() ));
   res.push_back(new command_child( "remote", new at_cmd_remote() ));

   return res;
};

command * setup_commands() {
   return new command_parent( "<global>", toplevel() );
}
