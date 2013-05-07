/* prefix_map_test.cpp
 *
 * Tests for the prefix map
 *
 * Author: Austin Hendrix
 */

#include "prefix_map.h"

#include <assert.h>
#include <stdio.h>

void p() {
   static int i = 0;
   printf("Test %d\n", i++);
}

int test() {
   prefix_map<int> test_m;
   int data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
   test_m.put("asdf", data);
   int * t;
   t = test_m.get("a");
   p();
   assert(t && *t == 1);

   t = test_m.get("as");
   p();
   assert(t && *t == 1);

   t = test_m.get("asd");
   p();
   assert(t && *t == 1);
   
   t = test_m.get("asdf");
   p();
   assert(t && *t == 1);

   t = test_m.get("asdfg");
   p();
   assert(!t);

   test_m.put("asfd", data+1);

   t = test_m.get("a");
   p();
   assert(!t);

   t = test_m.get("as");
   p();
   assert(!t);

   t = test_m.get("asd");
   p();
   assert(t && *t == 1);

   t = test_m.get("asf");
   p();
   assert(t && *t == 2);

   test_m.put("b", data+2);

   t = test_m.get("b");
   p();
   assert(t && *t == 3);

   t = test_m.get("c");
   p();
   assert(!t);

   t = test_m.get("bar");
   p();
   assert(!t);

   std::list<std::string> k;
   std::list<std::string>::iterator k_itr;

   p();
   k = test_m.get_keys("bar");
   // should be empty
   assert(k.size() == 0);
   
   p();
   k = test_m.get_keys("");
   // should contain "asdf", "asfd", "b"
   assert(k.size() == 3);
   k_itr = k.begin();
   assert(*k_itr == "asdf");
   k_itr++;
   assert(*k_itr == "asfd");
   k_itr++;
   assert(*k_itr == "b");
   
   p();
   k = test_m.get_keys("b");
   // should contain "b"
   assert(k.size() == 1);
   k_itr = k.begin();
   assert(*k_itr == "b");

   p();
   k = test_m.get_keys("a");
   // should contain "asdf", "asfd"
   assert(k.size() == 2);
   k_itr = k.begin();
   assert(*k_itr == "asdf");
   k_itr++;
   assert(*k_itr == "asfd");

}

int main(int argc, char ** argv) {
   test();
}
