// AT commands (mapping to AT commands):
//
//  device type (ATDD)
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
//  apply (ATAC)
//  write (ATWR)
//  defaults (ATRE)
//  soft reset (ATFR)
//  network reset (ATNR)
//  comission (ATCB)
//  node discover (ATND)
//  resolve NI (ATDN)
//
//  TODO: move to I/O command set
//  force IO sample (ATIS)
//  force sensor sample (AT1S)


#ifndef XBEE_AT_H
#define XBEE_AT_H

#include <string>

// get the AT command for a given string
typedef std::string (*command_at)(std::string args);

#endif
