
// AT IO Commands
//  io sample-rate (ATID)
//  io change-detection (ATIC)
//  io led-blink-time (ATLT)
//  io RSSI-PWM (ATRP)
//  io temperature (ATTP) (PRO S2B only)
//
//  io config PWM0 (ATP0)
//  io config DIO11 (ATP1)
//  io config DIO12 (ATP2)
//  io config DIO13 (ATP3)
//  io config AD0/DIO0 (ATD0)
//  io config AD1/DIO1 (ATD1)
//  io config AD2/DIO2 (ATD2)
//  io config AD3/DIO3 (ATD3)
//  io config DIO4 (ATD4)
//  io config DIO5 (ATD5)
//  io config DIO8 (ATD8)
//  io config pull-up (ATPR)
//
//  io voltage supply (AT%V)
//  io voltage monitoring (ATV+)
//
//  io force-sample (ATIS)
//  TODO: io sensor-sample (AT1S)
//

#include "xbee_at_cmd.h"

std::list<command*> io();
