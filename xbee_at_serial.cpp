
#include "xbee_at_cmd.h"

command ** serial_api() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "escape",  new at_cmd_enum("AP", 2,
            1, "normal",
            2, "escape"
            ));
   *r++ = new command_child( "options", new at_cmd_enum("AO", 3,
            0, "default",
            1, "explicit-rx",
            3, "zdo"
            ));
   *r++ = 0;

   return result;
}

command ** serial_c() {
   command ** result = new command*[8];
   command ** r = result;
   *r++ = new command_parent( "api", serial_api());

   *r++ = new command_child( "baud", new at_cmd_enum("BD", 8,
            0, "1200",
            1, "2400",
            2, "4800",
            3, "9600",
            4, "19200",
            5, "38400",
            6, "57600",
            7, "115200"
            ));
   *r++ = new command_child( "parity", new at_cmd_enum("NB", 4,
            0, "none",
            1, "even",
            2, "odd",
            3, "mark"
            ));
   *r++ = new command_child( "stop-bits", new at_cmd_enum("SB", 2,
            0, "1",
            1, "2"
            ));
   *r++ = new command_child( "packetization-timeout", fake_cmd);
   *r++ = new command_child( "DIO6", new at_cmd_enum("D7", 7,
            0, "disabled",
            1, "cts",
            3, "digital-in",
            4, "off",
            5, "on",
            6, "rs485-tx-enable-low",
            7, "rs485-tx-enable-high"
            ));
   *r++ = new command_child( "DIO7", new at_cmd_enum("D6", 5,
            0, "disabled",
            1, "rts",
            3, "digital-in",
            4, "off",
            5, "on"
            ));
   *r++ = 0;

   return result;
}



