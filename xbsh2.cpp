/* xcsh2.cpp
 *
 * New XBee shell
 *
 * Author: Austin Hendrix
 */


#include <serial/serial.h>

#include "xbee_api.h"
#include "prefix_map.h"

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics
typedef int (*command_f)(char * args);

// commands:
//  remote mode: switch to remote AT commands. pushes address onto remote stack
//  exit: leave current remote node; or exit completely
//
// API frames:
//  see xbee_api.h
//
// AT commands (mapping to AT commands):
//  destination (ATDL + ATDH)
//  16-bit address (ATMY)
//  16-bit parent  (ATMP)
//  children (ATNC)
//  serial (ATSL + ATSH)
//  node ID (ATNI)
//  cluster ID (ATCI)
//  max payload (ATNP)
//  device type (ATDD)
//  channel (ATCH)
//  extended PAN ID (ATID) - PAN ID to join (0 for any)
//  operating PAN ID (ATOP) - current PAN ID
//  maximum unicast hops (ATNH)
//  maximum broadcast hops (ATBH)
//  16-bit PAN ID (ATOI)
//  node discovery timeout (ATNT)
//  node discovery options (ATNO)
//  scan channels (ATSC)
//  scan duration (ATSD)
//  zigbee profile (ATZS)
//  join time (ATNJ)
//  channel verification (ATJV)
//  join notification (ATJN)
//
//  encryption enable (ATEE)
//  encryption options (ATEO)
//  encryption key (ATNK)
//  encryption link key (ATKY)
//
//  RF power level (ATPL)
//  RF power mode (ATPM)
//  RF RSSI (ATDB)
//  RF peak power (ATPP)
//
//  serial API escape (ATAP)
//  serial API options (ATAO)
//  serial baud (ATBD)
//  serial parity (ATNB)
//  serial stop bits (ATSB)
//  serial packetization timeout (ATRO)
//  serial DIO7 config (ATD7)
//  serial DIO6 config (ATD6)
//
//  io sample rate (ATID)
//  io digital change detection (ATIC)
//  io PWM0 config (ATP0)
//  io DIO11 config (ATP1)
//  io DIO12 config (ATP2)
//  io DIO13 config (ATP3)
//  io AD0/DIO0 config (ATD0)
//  io AD1/DIO1 config (ATD1)
//  io AD2/DIO2 config (ATD2)
//  io AD3/DIO3 config (ATD3)
//  io DIO4 config (ATD4)
//  io DIO5 config (ATD5)
//  io DIO8 config (ATD8)
//  io led blink time (ATLT)
//  io pull-up config (ATPR)
//  io RSSI PWM (ATRP)
//  io supply voltage (AT%V)
//  io voltage monitoring (ATV+)
//  io module temperature (ATTP) (PRO S2B only)
//
//  diag FW version (ATVR)
//  diag HW version (ATHV)
//  diag associate status (ATAI)
//
//  AT command mode timeout (ATCT)
//  AT end command mode (ATCN)
//  AT guard time (ATGT)
//  AT command character (ATCC)
//
//  sleep mode (ATSM)
//  sleep period count (ATSN)
//  sleep period time (ATSP)
//  sleep timeout (ATST)
//  sleep options (ATSO)
//  sleep wake host (ATWH)
//  sleep now (ATSI)
//  sleep poll rate (ATPO)
//
//  apply (ATAC)
//  write (ATWR)
//  defaults (ATRE)
//  soft reset (ATFR)
//  network reset (ATNR)
//  comission (ATCB)
//  node discover (ATND)
//  resolve NI (ATDN)
//  force IO sample (ATIS)
//  force sensor sample (AT1S)

int main(int argc, char ** argv) { 
   prefix_map<command_f> commands;
   //commands.put(
   return 0;
}
