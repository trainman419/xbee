/* xcsh2.cpp
 *
 * New XBee shell
 *
 * Author: Austin Hendrix
 */

#include <string>

#include <assert.h>
#include <stdio.h>
#include <serial/serial.h>

#include "xbee_api.h"

// every shell command is a function
//  args is the remainder of the line, with leading spaces stripped
//  the return code follows standard POSIX return code semantics
typedef int (*command_f)(char * args);

class command_t {
   public:
      char * name;
      command_f command;
};

template <class T> class prefix_map {
   public:
      prefix_map() : map(NULL) {}

      // get an object by prefix
      T * get(std::string prefix) {
         int depth = 0;
         prefix_node * t = map;
         while(t) {
            if(t->prefix == prefix[depth]) {
               if(t->leaf) {
                  // we found a leaf node that matches
                  prefix_obj * o = t->child;

                  // check that our prefix is long enough
                  if( prefix.length() <= o->name.length() ) {
                     // check that our prefix matches
                     if( o->name.substr(0, prefix.length()) == prefix ) {
                        return o->obj;
                     } else {
                        // no prefix match; done
                        return NULL;
                     }
                  } else {
                     return NULL;
                  }
               } else {
                  t = t->children;
                  depth++;
                  if( depth >= prefix.length() ) {
                     // we ran out of prefix before tree traversal was done
                     //  TODO: return a better or error, or maybe all of the
                     //  potential results
                     return NULL;
                  }
               }
            } else {
               t = t->next;
            }
         }
         return NULL;
      }

      // insert an object by full ID
      void put(std::string id, T * o) {
         prefix_obj * obj = new prefix_obj(id, o);
         if(map) {
            int depth = 0;
            prefix_node * n = map;
            prefix_node * prev = map;
            while(n) {
               if( id[depth] == n->prefix ) {
                  if(n->leaf) {
                     prefix_obj * obj2 = n->child;
                     n->leaf = 0;
                     // determine depth
                     int i = 0;
                     for( ; i < obj->name.length() && 
                           i < obj2->name.length() && 
                           obj->name[i] == obj2->name[i]; i++ );

                     for( depth++; depth < i; depth++ ) {
                        n->children = new prefix_node(id[depth], NULL);
                        n = n->children;
                     }
                     n->children = new prefix_node(id[depth], obj);
                     n = n->children;
                     n->next = new prefix_node(obj2->name[depth], obj2);
                     return;
                  } else {
                     n = n->children;
                     depth++;
                  }
               } else {
                  prev = n;
                  n = n->next;
               }
            }
            prev->next = new prefix_node(id[depth], obj);

         } else {
            map = new prefix_node(id[0], obj);
         }
      }

   private:
      class prefix_obj {
         public:
            prefix_obj(std::string & n, T * o) : name(n), obj(o) {}
            std::string name;
            T * obj;
      };

      class prefix_node {
         public:
            prefix_node(char c, prefix_obj * o) : prefix(c), child(o), 
               leaf(o?1:0), next(NULL) {}

            prefix_node * next;
            char prefix;
            union {
               prefix_node * children;
               prefix_obj * child;
            };
            int leaf;
      };

      prefix_node * map;
};

int test() {
   prefix_map<int> test_m;
   int data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
   test_m.put("asdf", data);
   int * t;
   t = test_m.get("a");
   printf("Test 1\n");
   assert(t && *t == 1);

   t = test_m.get("as");
   printf("Test 2\n");
   assert(t && *t == 1);

   t = test_m.get("asd");
   printf("Test 3\n");
   assert(t && *t == 1);
   
   t = test_m.get("asdf");
   printf("Test 4\n");
   assert(t && *t == 1);

   t = test_m.get("asdfg");
   printf("Test 5\n");
   assert(!t);

   test_m.put("asfd", data+1);

   t = test_m.get("a");
   printf("Test 6\n");
   assert(!t);

   t = test_m.get("as");
   printf("Test 7\n");
   assert(!t);

   t = test_m.get("asd");
   printf("Test 8\n");
   assert(t && *t == 1);

   t = test_m.get("asf");
   printf("Test 9\n");
   assert(t && *t == 2);

   test_m.put("b", data+2);

   t = test_m.get("b");
   printf("Test 10\n");
   assert(t && *t == 3);

   t = test_m.get("c");
   printf("Test 11\n");
   assert(!t);

   t = test_m.get("bar");
   printf("Test 12\n");
   assert(!t);
}

int main(int argc, char ** argv) { 
   test();
   return 0;
}
