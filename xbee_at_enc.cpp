
#include "xbee_at_enc.h"

#include <stdlib.h>
#include <ctime>

int rand_init = 0;

class at_cmd_key : public at_cmd_wo {
   public:
      at_cmd_key(std::string cmd) : at_cmd_wo(cmd) {}
      virtual ~at_cmd_key() {}

   protected:
      virtual std::vector<uint8_t> write_frame(xbsh_state * state, 
            std::string arg) {
         std::vector<uint8_t> ret(16, 0);
         std::vector<uint8_t> key;
         key.reserve(16);

         if( arg == "random" ) {
            // TODO: make this use a cryptographically secure random generator
            if( ! rand_init ) {
               srand((unsigned)time(NULL));
            }

            for( int i=0; i<16; i++ ) {
               ret[i] = rand();
            }
         } else {
            const char * arg_c = arg.c_str();
            for( int i=0 ; i < arg.length(); i += 2 ) {
               char tmp_c[3];
               tmp_c[0] = (arg_c + i)[0];
               tmp_c[1] = (arg_c + i)[1];
               tmp_c[2] = 0;

               uint8_t tmp;
               if( sscanf(tmp_c, "%hhX", &tmp) == 1 ) {
                  if( state->debug ) {
                     printf("Parsed %s as %02X\n", tmp_c, tmp);
                  }
                  key.push_back(tmp);
               } else {
                  printf("Error: Failed to parse %s as hex\n", arg.c_str());
                  ret.clear();
                  return ret;
               }
            }

            if( key.size() > 16 ) {
               printf("Error: key too long (%zd); should be 128 bits or less\n",
                     8 * key.size());
               ret.clear();
               return ret;
            }

            for( int i=16 - key.size(), j=0; i<16; i++, j++ ) {
               ret[i] = key[j];
            }
         }

         printf("New key: %02X", ret[0]);
         for( int i=1; i<16; i++ ) {
            printf(":%02X", ret[i]);
         }
         printf("\n");

         return ret;
      }
};

std::list<command*> enc() {
   std::list<command*> res;
   res.push_back(new command_child( "link-key", new at_cmd_key("KY")));
   res.push_back(new command_child( "enable", new at_cmd_enum("EE", 2,
            0, "on", 1, "off")));
   res.push_back(new command_child( "options", new at_cmd_flags("EO", 2, 
            "unsecured-key", "trust-center")));
   res.push_back(new command_child( "key", new at_cmd_key("NK")));

   return res;
}
