/* prefix_map.h
 *
 * A write-only map class for storing object by keys and retrieving them by
 *  any unique prefix of that key.
 *
 * Author: Austin Hendrix
 */

#ifndef PREFIX_MAP_H
#define PREFIX_MAP_H

#include <string>
#include <list>
#include <queue>

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

      std::list<std::string> get_keys(std::string prefix) {
         std::list<std::string> result;
         prefix_node * n = map;
         int depth = 0;
         // recurse down to our prefix
         while(n && depth < prefix.length() ) {
            if(n->prefix == prefix[depth]) {
               if(n->leaf) {
                  // we found a leaf node that matches
                  prefix_obj * o = n->child;

                  // check that our prefix is long enough
                  if( prefix.length() <= o->name.length() ) {
                     // check that our prefix matches
                     if( o->name.substr(0, prefix.length()) == prefix ) {
                        result.push_back(n->child->name);
                     }
                  }
                  return result;
               } else {
                  n = n->children;
                  depth++;
               }
            } else {
               n = n->next;
            }
         }
         // gather all of the strings under our prefix
         std::queue<prefix_node*> nodes;
         if(n) {
            nodes.push(n);
         }
         while(!nodes.empty()) {
            n = nodes.front();
            nodes.pop();
            if(n->leaf) {
               result.push_back(n->child->name);
            } else {
               nodes.push(n->children);
            }
            if(n->next) {
               nodes.push(n->next);
            }
         }
         result.sort();
         return result;
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

#endif
