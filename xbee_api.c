/* implementation of my Xbee API in C

   Author: Austin Hendrix
 */
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include "xbee_api.h"

int is_escape(char b) {
   return b == 0x7E || b == 0x7D || b == 0x11 || b == 0x13;
}

/* overhead: start, size(2), checksum */
#define OVERHEAD 4
packet build_packet(int sz, char * data) {

   int escapes = count_escapes(sz, data);
   int i, offset;
   
   packet res;
   res.sz = sz + escapes + OVERHEAD;
   res.data = (char*)malloc(res.sz);

   /* TODO: escape size bytes and checksum */
   res.data[0] = 0x7E;
   res.data[1] = (sz >> 8) & 0xFF;
   res.data[2] = sz & 0xFF;

   char checksum = 0;
   offset = 3;
   for( i = 0; i < sz; i++ ) {
      if( is_escape(data[i]) ) {
         res.data[i + offset] = 0x7D;
         offset++;
         res.data[i + offset] = data[i] ^ 0x20;
      } else {
         res.data[i + offset] = data[i];
      }
      checksum += data[i];
   }

   checksum = 0xFF - checksum;
   res.data[i + offset] = checksum;

   return res;
}

/* count the number of characters that need to be escaped */
int count_escapes(int sz, char * data) {
   int i;
   int cnt = 0;
   for( i = 0; i < sz; i++ ) {
      if( is_escape(data[i]) ) cnt++;
   }
   return cnt;
}

packet wrap1(char type, char * data) {
   int sz = strlen(data);
   int i;

   char * tmp = malloc(sz + 1);
   
   tmp[0] = type;
   for( i = 0; i < sz; i++ ) {
      tmp[i+1] = data[i];
   }

   packet res = build_packet(sz + 1, tmp);

   /* we allocated a buffer, don't forget to free it */
   free(tmp);

   return res;
}

packet at(char * data) {
   static char cnt = 0;
   cnt++;
   int sz = strlen(data);
   int i;

   char * tmp = malloc(sz + 2);
   
   tmp[0] = API_AT;;
   tmp[1] = cnt;
   for( i = 0; i < sz; i++ ) {
      tmp[i+2] = data[i];
   }

   packet res = build_packet(sz + 2, tmp);

   /* we allocated a buffer, don't forget to free it */
   free(tmp);

   return res;
}

packet at_queue(char * data) {
   return wrap1(API_AT_QUEUE, data);
}

packet remote_at(xbee_addr addr, xbee_net net, char * data) {
   packet res;
   int i;
   int sz = 1 + 1 + 8 + 2 + 1 + strlen(data);
   static char cnt = 0;
   cnt++;

   unsigned char * buf = (unsigned char *)malloc(sz);

   /* API Identifier */
   buf[0] = API_REMOTE_AT;
   /* Frame ID */
   buf[1] = cnt;
   /* Destination address */
   for( i = 0; i < 8; i++ ) buf[i+2] = addr.c_addr[i];
   /* Destination network */
   for( i = 0; i < 2; i++ ) buf[i+10] = net.c_net[i];

   /* Command Options */
   buf[12] = 0x02; /* TODO: make this modifiable; currently, just apply 
                      immediately */

   for( i = 0; i < strlen(data); i++ ) buf[i+13] = data[i];

   res = build_packet(sz, buf);

   free(buf);

   return res;
}

/* read a complete packet from a file descriptor */
/* if this gets a packet start, it will loop until it gets a complete packet */
packet read_packet(int fd) {
   packet res;
   int bufsz = 256;
   unsigned char * data = (unsigned char *)malloc(bufsz);

   int sz;
   int i, j;

   res.data = (unsigned char*)malloc(bufsz);
   res.sz = 0;

   /* TODO: unescape incoming data */

   res.sz = read(fd, res.data, OVERHEAD);
   if( res.sz > 0 ) {

      /* find packet start */
      for( i = 0; i < res.sz && res.data[i] != 0x7E; i++ ) {
         printf("stray byte %02X\n", res.data[i]);
      }

      if( i < res.sz ) {
         /* shuffle our packet data forward and resize */
         for( j = i; j < res.sz; j++ ) res.data[j - i] = res.data[j];
         res.sz -= i; 

         int inc = 1;
         int escape = 0;
         for( i=1; i<res.sz; i++ )
            if( res.data[i] == 0x7D )
               escape++;

         /* read in enough data to figure out something about this packet */
         while( res.sz < (OVERHEAD + escape) ) {
            inc = read(fd, res.data + res.sz, (OVERHEAD + escape) - res.sz);
            for( i=0; i < inc; i++ )
               if( res.data[res.sz+i] == 0x7D )
                  escape++;
            res.sz += inc;
         }

         /* deal with escapes in length fields */
         while( escape > 0 ) {
            /* find our escape character */
            int start;
            for( start = 0; res.data[start] != 0x7D; start++ );
            res.data[start] = res.data[start+1] ^ 0x20;
            res.sz--;
            for( i = start + 1; i < res.sz; i++ )
               res.data[i] = res.data[i+1];
            escape--;
         }
         sz = res.data[2] | (res.data[1] << 8);

         /* read the rest of our packet */
         while( res.sz < (OVERHEAD + sz + escape) && inc > 0 ) {
            inc = read(fd, res.data + res.sz, (sz + OVERHEAD + escape)
                  - res.sz); 
            for( i=0; i<inc; i++ ) {
               if( res.data[i+res.sz] == 0x7D ) escape++;
            }
            res.sz += inc;
         }

         if( res.sz < (OVERHEAD + sz + escape) ) {
            printf("Packet short by %d bytes\n", (OVERHEAD + sz + escape) - 
                  res.sz);
         }

         while( escape > 0 ) {
            /* find our escape character */
            int start;
            for( start = 0; res.data[start] != 0x7D; start++ );
            res.data[start] = res.data[start+1] ^ 0x20;
            res.sz--;
            for( i = start + 1; i < res.sz; i++ )
               res.data[i] = res.data[i+1];
            escape--;
         }

         return res;
      }
   } 

   // looks like we didn't get anything. return an empty packet.
   res.sz = 0;
   free(res.data);
   res.data = 0;
   return res;
}

/* parse a packet and print it to stdout with stdio */
void print_packet(packet p) {
   int len = p.data[2] | (p.data[1] << 8);
   int i;

   switch(p.data[3]) {
      case API_AT_RESP:
         printf("AT Command response %d: %c%c %s\n", p.data[4], p.data[5],
               p.data[6], p.data[7]==0?"OK":(p.data[7]==1?"Error":
                  (p.data[7]==2?"Invalid Command":"Invalid Parameter")));
         printf("Data: ");
         for( i = 8; i < (p.sz -1); i++ ) printf("%02X ", p.data[i]);
         printf("\n");
         break;
      case API_REMOTE_AT_RESP:
         printf("Remote AT Command response %c%c %s\n", p.data[15], p.data[16], 
               p.data[17]==0?"OK":(p.data[17]==1?"Error":
                  (p.data[17]==2?"Invalid Command":"Invalid Parameter")));
         printf("Address: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
               p.data[5], p.data[6], p.data[7], p.data[8], p.data[9],
               p.data[10], p.data[11], p.data[12]);
         printf("Network: %02X:%02X\n", p.data[13], p.data[14]);
         printf("Data: ");
         for( i = 18; i < (p.sz -1); i++ ) printf("%02X ", p.data[i]);
         printf("\n");
         break;
      default:
         printf("Unrecognized packet type: %02X\n", p.data[3]);
         for( i = 3; i < p.sz; i++ ) printf("%02X ", p.data[i]);
         printf("\n");
         break;
   }
}
