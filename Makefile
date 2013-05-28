CFLAGS=-g
CXXFLAGS=-g
LDLIBS=-lreadline -lserial
LINK.o=$(CXX)

TARGETS=xbsh sign xbsh2 prefix_map_test
OBJS=xbee_api.o xbee_at_cmd.o

all: $(TARGETS)
.PHONY: all

xbsh: xbsh.o xbee_api.o

xbsh2: xbsh2.o xbee_api.o xbee_at_cmd.o xbee_at.o xbee_at_io.o

sign: sign.o xbee_api.o

clean:
	-rm -rf $(TARGETS) *.o
.PHONY: clean

test: prefix_map_test xbsh2
	./prefix_map_test	
	./xbsh2 < test_xbsh2_1.in | diff -u --strip-trailing-cr - test_xbsh2_1.out
.PHONY: test

.%.mk: %.c
	$(CC) -MMD -c $^ -MF $@

.%.mk: %.cpp
	$(CXX) -MMD -c $^ -MF $@

include $(TARGETS:%=.%.mk)
include $(OBJS:%.o=.%.mk)
