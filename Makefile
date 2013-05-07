CFLAGS=-g
CXXFLAGS=-g
LDFLAGS=-lreadline -lserial

OBJS=xbsh sign xbsh2 prefix_map_test

all: $(OBJS)
.PHONY: all

xbsh: xbsh.o xbee_api.o

xbsh.o: xbee_api.h

xbsh2: xbsh2.o xbee_api.o

xbsh2.o: xbee_api.h prefix_map.h

xbee_api.o: xbee_api.h

prefix_map_test: prefix_map.h

sign: sign.o xbee_api.o

sign.o: xbee_api.h

clean:
	-rm -rf $(OBJS) *.o
.PHONY: clean

test: prefix_map_test
	./prefix_map_test	
.PHONY: test
