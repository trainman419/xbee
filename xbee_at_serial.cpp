
#include "xbee_at_serial.h"

std::list<command*> serial_api() {
   std::list<command*> res;
   res.push_back(new command_child( "escape",  new at_cmd_enum("AP", 2,
            1, "normal",
            2, "escape"
            )));
   res.push_back(new command_child( "options", new at_cmd_enum("AO", 3,
            0, "default",
            1, "explicit-rx",
            3, "zdo"
            )));

   return res;
}

std::list<command*> serial_c() {
   std::list<command*> res;
   res.push_back(new command_parent( "api", serial_api()));

   res.push_back(new command_child( "baud", new at_cmd_enum("BD", 8,
            0, "1200",
            1, "2400",
            2, "4800",
            3, "9600",
            4, "19200",
            5, "38400",
            6, "57600",
            7, "115200"
            )));
   res.push_back(new command_child( "parity", new at_cmd_enum("NB", 4,
            0, "none",
            1, "even",
            2, "odd",
            3, "mark"
            )));
   res.push_back(new command_child( "stop-bits", new at_cmd_enum("SB", 2,
            0, "1",
            1, "2"
            )));
   res.push_back(new command_child( "packetization-timeout", 
            new at_cmd_scaled("RO", 1, 0, 0xFF, 1, "character times",
               "Packetization timeout")));
   res.push_back(new command_child( "DIO6", new at_cmd_enum("D7", 7,
            0, "disabled",
            1, "cts",
            3, "digital-in",
            4, "off",
            5, "on",
            6, "rs485-tx-enable-low",
            7, "rs485-tx-enable-high"
            )));
   res.push_back(new command_child( "DIO7", new at_cmd_enum("D6", 5,
            0, "disabled",
            1, "rts",
            3, "digital-in",
            4, "off",
            5, "on"
            )));

   return res;
}
