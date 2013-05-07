CFLAGS=-g
CXXFLAGS=-g
LDFLAGS=-lreadline -lserial

TARGETS=xbsh sign xbsh2 prefix_map_test
OBJS=xbee_api.o xbee_at_cmd.o

all: $(TARGETS)
.PHONY: all

xbsh: xbsh.o xbee_api.o

xbsh2: xbsh2.o xbee_api.o xbee_at_cmd.o

sign: sign.o xbee_api.o

clean:
	-rm -rf $(TARGETS) *.o .*.mk
.PHONY: clean

test: prefix_map_test
	./prefix_map_test	
.PHONY: test

.%.mk: %.c
	$(CC) -MM $^ -MF $@

.%.mk: %.cpp
	$(CXX) -MM $^ -MF $@

include $(TARGETS:%=.%.mk)
include $(OBJS:%.o=.%.mk)
