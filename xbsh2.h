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
#include <stdint.h>

class api_frame {
   protected:
      uint8_t type;
      uint8_t id;
      uint8_t status;
      std::string command;
      std::vector<uint8_t> data;

   public:
      api_frame( uint8_t t, uint8_t i, uint8_t s, std::string c,
          std::vector<uint8_t> d ) : 
        type(t), id(i), status(s), command(c), data(d) {}

      uint8_t get_type() { return type; }
      uint8_t get_id() { return id; }
      uint8_t get_status() { return status; }
      std::string get_command();
      std::vector<uint8_t> get_data() { return data; }
};

class api_remote_frame : public api_frame {
   protected:
      xbee_addr source;
      xbee_net net;

   public:
      api_remote_frame( uint8_t t, uint8_t i, uint8_t s, std::string c,
          xbee_addr so, xbee_net n, std::vector<uint8_t> d ) :
        api_frame(t, i, s, c, d), source(so), net(n) {}
      xbee_addr get_source() { return source; }
      xbee_net get_net() { return net; }
};

class xbsh_state {
   private:
      // serial port
      serial::Serial serial;
      std::vector<uint8_t> partial;
      
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
      api_frame * read_AT();

      // push and pop the remote address stack
      void push_remote(xbee_addr remote);
      xbee_addr get_remote();
      void pop_remote();
};

#endif