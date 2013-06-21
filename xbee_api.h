
/* RAW API commands, Digi XBee modules 
 * not all modules support all commands */

#ifndef XBEE_API_H
#define XBEE_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * API mode: AP = 2, API with escaped characters
 *
 */

/* generate a packet from packet data, adding escapes as needed
 *  mallocs a buffer and returns it; caller is responsible for freeing it
 *
 * This works for API versions 1 and 2/2.5
 */
typedef struct {
   int sz;
   unsigned char * data;
} packet;

packet build_packet(int sz, char * data);
/* count characters that need to be escaped */
int count_escapes(int sz, char * data);

void print_packet(packet p);

/* 64-bit address. ought to be platform-agnostic */
typedef union {
   unsigned char c_addr[8];
   long unsigned int l_addr;
} xbee_addr;

typedef union {
   unsigned char c_net[2];
   short unsigned int s_net;
} xbee_net;

/* API commands */
#define API_TX64        0x00
#define API_TX16        0x01
#define API_AT          0x08
#define API_AT_QUEUE    0x09
#define API_TX          0x10 /* tx with broadcast */
#define API_EXPLICIT_TX 0x11
#define API_REMOTE_AT   0x17

/* API responses */
#define API_RX64        0x80
#define API_RX16        0x81
#define API_AT_RESP     0x88
#define API_TX_STATUS   0x89
#define API_STATUS      0x8A
#define API_TX_STATUS2  0x8B /* I think this is from version 1 */
#define API_RX          0x90
#define API_EXPLICIT_RX 0x91
#define API_IO_RX       0x92
#define API_SENSOR_RX   0x94
#define API_NODE_IDENTIFY 0x95
#define API_REMOTE_AT_RESP 0x97

/* various overloaded transmit functions */
packet tx(int sz, char * data);
packet tx_addr(xbee_addr addr, int sz, char * data);

/* reception... 
 *
 * single entry point?
 * static input structures?
 * remember last set of inputs?
 */
/* I'm going to avoid this, since mostly right now I need remote commands */
/* read a packet from the given file descriptor */
packet read_packet(int fd);

/* AT */
packet at(char * data, int data_sz);
packet at_queue(char * data, int data_sz);

/* remote AT */
packet remote_at(xbee_addr addr, xbee_net net, char * data, int data_sz);
/* not sure if there's a remote AT queue; skipping for now */

#ifdef __cplusplus
}
#endif


#endif
