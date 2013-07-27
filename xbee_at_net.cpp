
#include "xbee_at_cmd.h"

#include <boost/foreach.hpp>

class at_cmd_serial : public at_cmd_ro {
   public:
      at_cmd_serial() : at_cmd_ro("") {}

   protected:
      virtual int read(xbsh_state * state) {
         state->send_AT("SH");
         api_frame * high = state->read_AT();
         if( !high ) {
            printf("Didn't get a response\n");
            return 1;
         }
         if( !high->ok() ) {
            printf("Error: %s\n", high->get_error().c_str());
            return 2;
         }
         state->send_AT("SL");
         api_frame * low = state->read_AT();
         if( !low ) {
            printf("Didn't get a response\n");
            return 1;
         }
         if( !low->ok() ) {
            printf("Error: %s\n", low->get_error().c_str());
            return 2;
         }
         std::vector<uint8_t> high_data = high->get_data();
         std::vector<uint8_t> low_data  = low->get_data();
         if( high_data.size() == 4 && low_data.size() == 4 ) {
            printf("Serial: %02X%02X:%02X%02X:%02X%02X:%02X%02X\n",
                  high_data[0], high_data[1], high_data[2], high_data[3],
                  low_data[0],  low_data[1],  low_data[2],  low_data[3]);
            return 0;
         } else {
            printf("Expected 8 and 8 bytes, got %zd and %zd\n", 
                  high_data.size(), low_data.size());
            return 3;
         }
      }
};

class at_cmd_ni : public at_cmd_rw {
   public:
      at_cmd_ni() : at_cmd_rw("NI") {}

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret) {
         std::vector<uint8_t> data = ret->get_data();
         std::string out;
         out.reserve(data.size());
         BOOST_FOREACH(uint8_t c, data) {
            out.push_back(c);
         }
         printf("Node Idnetifier: %s\n", out.c_str());
         return 0;
      }

      virtual std::vector<uint8_t> write_frame(xbsh_state * state,
            std::string arg) {
         std::vector<uint8_t> ret;
         if( arg.length() <= 20 ) {
            ret.reserve(arg.length());
            BOOST_FOREACH(uint8_t c, arg) {
               ret.push_back(c);
            }
         } else {
            printf("Error: node identifier cannot be more "
                   "than 20 characters\n");
         }
         return ret;
      }
};

command ** net_short() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "address", new at_cmd_ro_hex("MY", "16-bit network address", 2) );
   *r++ = new command_child( "parent",  new at_cmd_ro_hex("MP", "16-bit parent address", 2) );
   *r++ = 0;
   return result;
};

command ** net_max() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "unicast", new at_cmd_scaled("NH", 1, 0, 0xFF,
            1, "hops", "Maximum unicast distance"));
   *r++ = new command_child( "broadcast", new at_cmd_scaled("BH", 1, 0, 0x1E,
            1, "hops", "Maximum broadcast distance"));
   *r++ = 0;
   return result;
};

command ** net_pan() {
   command ** result = new command*[4];
   command ** r = result;
   *r++ = new command_child( "operating", new at_cmd_ro_hex("OP", "64-bit Operating PAN ID", 8) );
   *r++ = new command_child( "long",      fake_cmd );
   *r++ = new command_child( "short", new at_cmd_ro_hex("OI", "16-bit Operating PAN ID", 2) );
   *r++ = 0;
   return result;
};

command ** net_node() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "timeout", new at_cmd_scaled("NT", 1, 0x20, 0xFF,
            100, "ms", "Network discovery timeout"));
   *r++ = new command_child( "options", new at_cmd_flags("NO", 2,
            "device-type", "local-response"));
   *r++ = 0;
   return result;
};

command ** net_join() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "notification", new at_cmd_enum("JN", 2,
            0, "off", 1, "on"));
   *r++ = new command_child( "time", new at_cmd_scaled("NJ", 1, 0, 0xFF, 1,
            "sec", "Node join time"));
   *r++ = 0;
   return result;
};

command ** net() {
   command ** result = new command*[11];
   command ** r = result;
   *r++ = new command_parent( "short",          net_short() );
   *r++ = new command_parent( "max-hops",       net_max()   );
   *r++ = new command_parent( "PAN-ID",         net_pan()   );
   *r++ = new command_parent( "node-discovery", net_node()  );
   *r++ = new command_parent( "join",           net_join()  );

   *r++ = new command_child( "node-id", new at_cmd_ni() );
   *r++ = new command_child( "max-payload", fake_cmd );
   *r++ = new command_child( "destination", fake_cmd );
   *r++ = new command_child( "children",    fake_cmd );
   *r++ = new command_child( "serial", new at_cmd_serial() );
   *r++ = 0;
   return result;
};
