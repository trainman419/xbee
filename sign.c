#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>

#include "xbee_api.h"

/* remote address 00 13 A2 00 40 3B 48 3B */
/* network 0xFFFE */
/* Pin mappings:
 *
 * D0:   5: hand on
 *       4: hand off
 * D1:   5: man off
 *       4: man on (override)
 */

#define TRIES 10

int serial_open(char * device);

int main(int argc, char ** argv) {
   xbee_addr addr;
   /* ick; sadly, no better way to set up our address */
   addr.c_addr[0] = 0x00;
   addr.c_addr[1] = 0x13;
   addr.c_addr[2] = 0xA2;
   addr.c_addr[3] = 0x00;
   addr.c_addr[4] = 0x40;
   addr.c_addr[5] = 0x3B;
   addr.c_addr[6] = 0x48;
   addr.c_addr[7] = 0x3B;

   /* set up our network address */
   xbee_net net;
   net.c_net[0] = 0xFF;
   net.c_net[1] = 0xFE;

   /* default device /dev/ttyUSB0 */
   char * dev = "/dev/ttyUSB0";
   int ser = serial_open(dev);

   if( ser < 0 ) return -1;

   char d0[] = {'D', '0', 0, 0};
   char d1[] = {'D', '1', 0, 0};

   /* interpret CLI options */
   int i;
   for( i=1; i < argc; i++ ) {
      char * arg = argv[i];
      if( strcmp("-man", arg) == 0 ) {
         d0[2] = 4;
         d1[2] = 4;
      }
      if( strcmp("-hand", arg) == 0 ) {
         d0[2] = 5;
         d1[2] = 5;
      }
      if( strcmp("-off", arg) == 0 ) {
         d0[2] = 4;
         d1[2] = 5;
      }
   }

   packet p;
   packet out;
   int retry = 0;
   int cnt = 0;
   unsigned char result[2] = { 0, 0};

   p.sz = 0;
   out = remote_at(addr, net, d0);
   while( p.sz == 0 && retry < TRIES ) {
      cnt = write(ser, out.data, out.sz);
      if( cnt != out.sz ) {
         //printf("Wrote %d of %d bytes\n", cnt, out.sz);
      }
      p = read_packet(ser);
      if( p.sz > 0 ) {
         //print_packet(p);
         result[0] = p.data[18];
      }
      //printf("Bit 0: Try %d\n", retry);
      retry++;
   }

   retry = 0;
   p.sz = 0;
   out = remote_at(addr, net, d1);
   while( p.sz == 0 && retry < TRIES ) {
      cnt = write(ser, out.data, out.sz);
      if( cnt != out.sz ) {
         //printf("Wrote %d of %d bytes\n", cnt, out.sz);
      }
      p = read_packet(ser);
      if( p.sz > 0 ) {
         //print_packet(p);
         result[1] = p.data[18];
      }
      //printf("Bit 1: Try %d\n", retry);
      retry++;
   }

   if( d0[2] == 0 ) {
      if( result[1] == 4 ) {
         printf("man\n");
      } else if( result[0] == 5 ) {
         printf("hand\n");
      } else {
         printf("off\n");
      }
   }
}

/* open a serial port and set baud rate to 115200 */
int serial_open(char * device) {
   int ser;
   struct termios newtio;

   ser = open(device, O_RDWR | O_NOCTTY); // open read-write, do not adopt as
   // controlling tty
   if( ser < 0) {
      perror(device);
      return -1;
   }

   bzero(&newtio, sizeof(struct termios)); // zero new port settings
   // set up new port 
   // see http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html)
   newtio.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   newtio.c_iflag = IGNBRK;
   newtio.c_oflag = 0; // raw output
   newtio.c_lflag = 0; // raw input
   // ignore control characters. block until at least 1 character received
   newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
   newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
   newtio.c_cc[VERASE]   = 0;     /* del */
   newtio.c_cc[VKILL]    = 0;     /* @ */
   newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
   newtio.c_cc[VTIME]    = 1;     /* inter-character timer deciseconds */
   newtio.c_cc[VMIN]     = 0;     /* blocking read until n characters arrive */
   newtio.c_cc[VSWTC]    = 0;     /* '\0' */
   newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
   newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
   newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
   newtio.c_cc[VEOL]     = 0;     /* '\0' */
   newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
   newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
   newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
   newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
   newtio.c_cc[VEOL2]    = 0;     /* '\0' */

   tcflush(ser, TCIFLUSH); // flush line
   tcsetattr(ser, TCSANOW, &newtio); // set terminal settings

   return ser;
}
