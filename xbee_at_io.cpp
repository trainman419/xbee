
#include "xbee_at_cmd.h"

#include <boost/foreach.hpp>

class at_cmd_sample : public at_cmd_ro {
   public:
      at_cmd_sample() : at_cmd_ro("IS") {}

   protected:
      virtual int read_frame(xbsh_state * state, api_frame * ret) {
         // parse sample frame
         std::vector<uint8_t> data = ret->get_data();
         if( data.size() >= 4 ) {
            io_sample sample(data);

            BOOST_FOREACH( const io_sample::digital d, sample.get_digital() ) {
               printf("DIO%d: %d\n", d.channel, d.data);
            }

            BOOST_FOREACH( const io_sample::analog a, sample.get_analog() ) {
               printf("AD%d: %d\n", a.channel, a.data);
            }
            
            return 0;
         } else {
            printf("Didn't get any sensor data\n");
            return 1;
         }
      }
};

command ** io_voltage() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "supply",     fake_cmd);
   *r++ = new command_child( "monitoring", fake_cmd);
   *r++ = 0;
   return result;
};

command ** io() {
   command ** result = new command*[24];
   command ** r = result;
   *r++ = new command_parent( "voltage",   io_voltage());

   *r++ = new command_child( "pull-up", new at_cmd_flags("PR",14,
            "D4", "D3", "D2", "D1", "D0", "D6", "D8", "DIN", "D5",
            "D9", "D12", "D10", "D11", "D7"));
   *r++ = new command_child( "PWM0", new at_cmd_enum("P0", 5,
            0, "disabled",
            1, "RSSI",
            3, "digital-in",
            4, "off",
            5, "on"));

   // DIO
   at_cmd * dio0 = new at_cmd_enum("D0", 5, 
         1, "comissioning",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio1 = new at_cmd_enum("D1", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio2 = new at_cmd_enum("D2", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");
   at_cmd * dio3 = new at_cmd_enum("D3", 5,
         0, "disabled",
         2, "analog-in",
         3, "digital-in",
         4, "off",
         5, "on");

   // Digital commands
   *r++ = new command_child( "D0",    dio0);
   *r++ = new command_child( "D1",    dio1);
   *r++ = new command_child( "D2",    dio2);
   *r++ = new command_child( "D3",    dio3);

   *r++ = new command_child( "D4",    new at_cmd_enum("D4", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   *r++ = new command_child( "D5",    new at_cmd_enum("D5", 5,
            0, "disabled",
            1, "associate",
            3, "digital-in",
            4, "off",
            5, "on"));

   // DIO6-7 are serial

   /* D8 is not yet supported 
   *r++ = new command_child( "D8",    new at_cmd_enum("D8", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
            */

   // DIO9-10 are serial
   *r++ = new command_child( "D11",    new at_cmd_enum("P1", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   *r++ = new command_child( "D12",    new at_cmd_enum("P2", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
   /*
   *r++ = new command_child( "D13",    new at_cmd_enum("P3", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
            */

   // A2D
   *r++ = new command_child( "AD0",      dio0);
   *r++ = new command_child( "AD1",      dio1);
   *r++ = new command_child( "AD2",      dio2);
   *r++ = new command_child( "AD3",      dio3);

   // other configurataion options
   *r++ = new command_child( "sample-rate", new at_cmd_scaled("IR", 2, 0x32, 
            0xFFFF, 1.0, "ms", "Sample rate")); // TODO: set 0 to mean off
   *r++ = new command_child( "change-detection", fake_cmd);
   *r++ = new command_child( "led-blink-time", new at_cmd_scaled("LT", 1, 0x0A,
            0xFF, 10.0, "ms", "LED blink time")); // TODO: set 0 to mean default
   *r++ = new command_child( "RSSI-PWM", new at_cmd_scaled("RP", 1, 0, 0xFF,
            100.0, "ms", "RSSI PWM")); // TODO: set 0xFF to mean always-on
   *r++ = new command_child( "temperature",      fake_cmd);
   *r++ = new command_child( "force-sample", new at_cmd_sample() );
   *r++ = 0;
   return result;
}
