
#include "xbee_at_cmd.h"

command ** io_voltage() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "supply",     fake_cmd);
   *r++ = new command_child( "monitoring", fake_cmd);
   *r++ = 0;
   return result;
};

command ** io() {
   command ** result = new command*[23];
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
   *r++ = new command_child( "D8",    new at_cmd_enum("D8", 4,
            0, "disabled",
            3, "digital-in",
            4, "off",
            5, "on"));
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
   *r++ = new command_child( "sample-rate",      fake_cmd);
   *r++ = new command_child( "change-detection", fake_cmd);
   *r++ = new command_child( "led-blink-time",   fake_cmd);
   *r++ = new command_child( "RSSI-PWM",         fake_cmd);
   *r++ = new command_child( "temperature",      fake_cmd);
   *r++ = 0;
   return result;
}
