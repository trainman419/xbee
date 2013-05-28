/* xbsh: the xbee command shell

   I'm starting this so that I have something that speaks the Xbee API, at
   least well enough to send AT commands to get out of API mode

   Author: Austin Hendrix
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

/* serial port stuff */
#include <fcntl.h>
#include <termios.h>

#include "xbee_api.h"

/* open a serial device and set up baud rate */
int serial_open(char * device);

int main(int argc, char ** argv) {
   int i;

   if( argc != 2 ) {
      printf("Usage: xbsh <device>\n");
      return 0;
   }

   int ser = serial_open(argv[1]);

   char * data = 0;

   unsigned char * input = (char*)malloc(64); /* input buffer */
   int in_sz = 0;
   packet p;
   xbee_addr addr;
   memset(addr.c_addr, 0, 8);

   /* I think this is the point-to-point network address */
   xbee_net net;
   net.c_net[0] = 0xFF;
   net.c_net[1] = 0xFE;

   do {
      /* free our buffer before we allocate stuff into it */
      free(data);
      data = readline("xbsh> ");
      /* AT command */
      if( data && strncmp( data, "AT", 2) == 0 ) {
         p = at(data + 2);
         write(ser, p.data, p.sz);
      }
      if( data && data[0] == 'r' ) {
         char * output = data + 1;
         if( strlen(data) > 3 ) {
            int param;
            output = malloc(strlen(data));
            output[0] = data[1];
            output[1] = data[2];
            sscanf(data + 3, "%d", &param);
            output[2] = param & 0xFF;
            output[3] = 0;
         }
         p = remote_at(addr, net, output);
         write(ser, p.data, p.sz);
         printf("Remote AT packet sent: %s\n", data);
         for( i=0; i<p.sz; i++ ) printf("%02X ", p.data[i]);
         printf("\n");
      }
      if( data && strncmp(data, "address", 7) == 0 ) {
         /* set or read the address */
         char * start = data;
         start += 7;
         while(isspace(*start)) start++;
         if( *start == 0 ) {
            /* read address */
            for( i=0; i<8; i++ )
               printf("%02X ", addr.c_addr[i]);
            printf("\n");
         } else {
            if( strlen(start) >= 16 ) { /* hex characters */
               printf("Set address to ");
               for( i=0; i<8; i++ ) {
                  sscanf(start, "%2hhX", addr.c_addr + i);
                  start += 2;
                  while( isspace(*start) ) start++;
                  printf("%02X ", addr.c_addr[i]);
               }
               printf("\n");
            } else {
               printf("Address too short\n");
            }
         }
      }

      /* done processing input; read data from the serial line */
      p = read_packet(ser);
      if( p.sz > 0 ) print_packet(p);

      /*while( (in_sz = read(ser, input, 64)) > 0 ) {
         for( i = 0; i < in_sz; i++ ) {
            printf("%02X ", input[i]);
         }
         printf(" | ");
         for( i = 0; i < in_sz; i++ ) {
            if( isprint(input[i]) ) {
               printf("%c", input[i]);
            } else {
               printf(".");
            }
         }
         printf("\n");
      }*/
   } while( data && strcmp(data, "exit") );

   free(data);
   return 0;
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
   //newtio.c_cc[VSWTC]    = 0;     /* '\0' */
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
