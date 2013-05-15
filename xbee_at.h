// AT commands (mapping to AT commands):
//
//  device type (ATDD)
//
//  diag FW-version (ATVR)
//  diag HW-version (ATHV)
//  diag associate-status (ATAI)
//
//  AT mode-timeout (ATCT)
//  AT end command mode (ATCN)
//  AT guard-time (ATGT)
//  AT command-character (ATCC)
//
//  reset soft (ATFR)
//  reset network (ATNR)
//  reset hard (toggle reset line)
//
//  discover-nodes (ATND)
//
//  apply (ATAC)
//  write (ATWR)
//  defaults (ATRE)
//  comission (ATCB)
//  resolve-NI (ATDN)
//
//  TODO: move to I/O command set
//  force IO sample (ATIS)
//  force sensor sample (AT1S)
//
//  TODO: shell buitins that don't correspond to AT commands:
//   show configuration
//   dump configuration to file
//   load config from file
//   change serial port parameters on-the-fly
//    - use case: update baud rate setting, update serial port to match
//    - serial rate unknown
//   hard reset via DTR line
//   firmware loading


#ifndef XBEE_AT_H
#define XBEE_AT_H

#include <string>

// get the AT command for a given string
typedef std::string (*command_at)(std::string args);

#endif
