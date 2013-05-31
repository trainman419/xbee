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
   p();
   int * t;
   t = test_m.get("a");
   p();
   assert( t );
   assert( *t == 1);

   t = test_m.get("as");
   p();
   assert( t );
   assert( *t == 1);

   t = test_m.get("asd");
   p();
   assert( t );
   assert( *t == 1);
   
   t = test_m.get("asdf");
   p();
   assert( t );
   assert( *t == 1);

   t = test_m.get("asdfg");
   p();
   assert(!t);

   test_m.put("asfd", data+1);
   p();

   t = test_m.get("a");
   p();
   assert(!t);

   t = test_m.get("as");
   p();
   assert(!t);

   t = test_m.get("asd");
   p();
   assert( t );
   assert( *t == 1);

   t = test_m.get("asf");
   p();
   assert( t );
   assert( *t == 2);

   p();
   test_m.put("asd", data+5);

   p();
   t = test_m.get("asd");
   assert( t );
   assert( *t == 6 );

   p();
   t = test_m.get("asdf");
   assert( t );
   assert( *t == 1 );

   p();
   test_m.put("asdfg", data+6);

   p();
   t = test_m.get("asd");
   assert( t );
   assert( *t == 6 );

   p();
   t = test_m.get("asdf");
   assert( t );
   assert( *t == 1 );

   p();
   t = test_m.get("asdfg");
   assert( t );
   assert( *t == 7 );

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
   assert(k.size() == 5);
   k_itr = k.begin();
   assert(*k_itr == "asd");
   k_itr++;
   assert(*k_itr == "asdf");
   k_itr++;
   assert(*k_itr == "asdfg");
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
   assert(k.size() == 4);
   k_itr = k.begin();
   assert(*k_itr == "asd");
   k_itr++;
   assert(*k_itr == "asdf");
   k_itr++;
   assert(*k_itr == "asdfg");
   k_itr++;
   assert(*k_itr == "asfd");

   p();
   k = test_m.get_keys("asd");
   assert(k.size() == 3);
   k_itr = k.begin();
   assert(*k_itr == "asd");
   k_itr++;
   assert(*k_itr == "asdf");
   k_itr++;
   assert(*k_itr == "asdfg");

   p();
   k = test_m.get_keys("c");
   // should be empty
   assert(k.size() == 0);

   test_m.put("xyzzy", data+3);
   test_m.put("xyaaa", data+4);

   p();
   k = test_m.get_keys("xy");
   assert(k.size() == 2);

   p();
   k = test_m.get_keys("xyz");
   assert(k.size() == 1);

   p();
   k = test_m.get_keys("xyzz");
   assert(k.size() == 1);

   return 0;
}

int main(int argc, char ** argv) {
   return test();
}
