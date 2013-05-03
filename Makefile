CFLAGS=-g
LDFLAGS=-lreadline -lserial

all: xbsh sign xbsh2
.PHONY: all

xbsh: xbsh.o xbee_api.o

xbsh.o: xbee_api.h

xbsh2: xbsh2.o xbee_api.o

xbsh2.o: xbee_api.h

xbee_api.o: xbee_api.h

sign: sign.o xbee_api.o

sign.o: xbee_api.h

clean:
	-rm -rf sign xbsh xbsh2 *.o
.PHONY: clean

