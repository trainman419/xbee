/* xcsh2.cpp
 *
 * New XBee shell
 *
 * Author: Austin Hendrix
 */

#include "xbee_api.h"
#include <serial/serial.h>

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics
typedef int (*command_f)(char * args);

class command_t {
   public:
      char * name;
      command_f command;
};

int main(int argc, char ** argv) { 
   return 0;
}
