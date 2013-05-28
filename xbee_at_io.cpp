
#include "xbee_at_cmd.h"

command ** io_config() {
   command ** result = new command*[17];
   command ** r = result;
   *r++ = new command_child( "pull-up", fake_cmd);
   *r++ = new command_child( "PWM0",    fake_cmd);

   // DIO
   *r++ = new command_child( "DIO00",    fake_cmd);
   *r++ = new command_child( "DIO01",    fake_cmd);
   *r++ = new command_child( "DIO02",    fake_cmd);
   *r++ = new command_child( "DIO03",    fake_cmd);
   *r++ = new command_child( "DIO04",    fake_cmd);
   *r++ = new command_child( "DIO05",    fake_cmd);
   // DIO6-7 are serial
   *r++ = new command_child( "DIO08",    fake_cmd);
   // DIO9-10 are serial
   *r++ = new command_child( "DIO11",    fake_cmd);
   *r++ = new command_child( "DIO12",    fake_cmd);
   *r++ = new command_child( "DIO13",    fake_cmd);

   // A2D
   *r++ = new command_child( "AD0",      fake_cmd);
   *r++ = new command_child( "AD1",      fake_cmd);
   *r++ = new command_child( "AD2",      fake_cmd);
   *r++ = new command_child( "AD3",      fake_cmd);
   
   *r++ = 0;
   return result;
}

command ** io_voltage() {
   command ** result = new command*[3];
   command ** r = result;
   *r++ = new command_child( "supply",     fake_cmd);
   *r++ = new command_child( "monitoring", fake_cmd);
   *r++ = 0;
   return result;
};

command ** io() {
   command ** result = new command*[8];
   command ** r = result;
   *r++ = new command_parent( "config",    io_config());
   *r++ = new command_parent( "voltage",   io_voltage());

   *r++ = new command_child( "sample-rate",      fake_cmd);
   *r++ = new command_child( "change-detection", fake_cmd);
   *r++ = new command_child( "led-blink-time",   fake_cmd);
   *r++ = new command_child( "RSSI-PWM",         fake_cmd);
   *r++ = new command_child( "temperature",      fake_cmd);
   *r++ = 0;
   return result;
}
