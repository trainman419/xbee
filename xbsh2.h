/* xbsh2.h
 *
 * Header for xbsh2; mostly defining the state classes
 *
 * Author: Austin Hendrix
 */

#ifndef XBSH2_H
#define XBSH2_H

#include "xbee_api.h"

#include <serial/serial.h>

#include <string>
#include <list>

class xbsh_state {
   private:
      // serial port
     serial::Serial serial;
      
      // remote addresses
      std::list<xbee_addr> remotes;
      
   public:
      xbsh_state(std::string port, int baud = 9600);

      // send and receive packets
      packet read_packet();
      void send_packet(packet p);

      // send AT command
      void send_AT(std::string at, char * data, int data_len);
      
      // receive AT command
      std::string read_AT();

      // push and pop the remote address stack
      void push_remote(xbee_addr remote);
      xbee_addr get_remote();
      void pop_remote();
};

#endif
