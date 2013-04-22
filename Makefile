CFLAGS=-g
LDFLAGS=-lreadline

all: xbsh sign

xbsh: xbsh.o xbee_api.o

xbsh.o: xbee_api.h

xbee_api.o: xbee_api.h

sign: sign.o xbee_api.o

sign.o: xbee_api.h
