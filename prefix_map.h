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
            //printf("%d: %c\n", depth, t->prefix);
            if(t->prefix == prefix[depth]) {
               if( depth+1 == prefix.length() ) {
                  if(t->child) {
                     prefix_obj * o = t->child;
                     if( prefix.length() <= o->name.length() ) {
                        return o->obj;
                     }
                  } else {
                     //printf("no child at prefix depth\n");
                     return NULL;
                  }
               } else if( depth+1 > prefix.length() ) {
                  // we ran out of prefix before tree traversal was done
                  //  this implies multiple matches
                  // TODO: return a better error message
                  //printf("prefix exhausted\n");
                  return NULL;
               } else {
                  if( t->children ) {
                     t = t->children;
                     depth++;
                  } else if( t->child ) {
                     // we've hit a leaf without hitting the end of our prefix
                     prefix_obj * o = t->child;
                     // check that our prefix is long enough
                     if( prefix.length() <= o->name.length() ) {
                        // check that our prefix matches
                        if( o->name.substr(0, prefix.length()) == prefix ) {
                           return o->obj;
                        }
                     }
                     // no prefix match; done
                     //printf("no prefix match\n");
                     return NULL;
                  } else {
                     //printf("no children and no child?\n");
                     return NULL;
                  }
               }
               /*
               // TODO: rework for new partial/full prefix overlap
               //  maybe invert prefix-length/depth check and child check
               if(t->child) {
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
               }
               if(t->children) {
                  t = t->children;
                  depth++;
                  if( depth >= prefix.length() ) {
                     // we ran out of prefix before tree traversal was done
                     //  TODO: return a better or error, or maybe all of the
                     //  potential results
                     return NULL;
                  }
               }
               */
            } else {
               t = t->next;
            }
         }
         //printf("fall-through\n");
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
               //printf("%d: %c\n", depth, id[depth]);
               if( id[depth] == n->prefix ) {
                  // TODO: rework this for new full/partial prefix overlap
                  if(n->children) {
                     n = n->children;
                     depth++;
                  } else {
                     prefix_obj * obj2 = n->child;
                     n->child = NULL;
                     // determine depth
                     int i = 0;
                     for( ; i < obj->name.length() && 
                           i < obj2->name.length() && 
                           obj->name[i] == obj2->name[i]; i++ );

                     //printf("m: %d (%s <> %s)\n", i, obj->name.c_str(), obj2->name.c_str());

                     if( i == id.length() ) {
                        n->child = obj;
                        //printf("prefix\n");
                        if( obj2->name.length() > i ) {
                           //printf("move child\n");
                           n->children = new prefix_node(obj2->name[i], obj2);
                        }
                        return;
                     } else {
                        for( depth++; depth < i; depth++ ) {
                           //printf("+%d: %c\n", depth, id[depth]);
                           n->children = new prefix_node(id[depth]);
                           n = n->children;
                        }
                        n->children = new prefix_node(id[depth], obj);
                        if( i == obj2->name.length() ) {
                           n->child = obj2;
                        } else {
                           n = n->children;
                           n->next = new prefix_node(obj2->name[depth], obj2);
                        }
                        return;
                     }
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
               if(n->child) {
                  // we found a leaf node that matches
                  prefix_obj * o = n->child;

                  // check that our prefix is long enough
                  if( prefix.length() <= o->name.length() ) {
                     // check that our prefix matches
                     if( o->name.substr(0, prefix.length()) == prefix ) {
                        result.push_back(n->child->name);
                     }
                  }
                  if(n->children) {
                     n = n->children;
                     depth++;
                  } else {
                     return result;
                  }
               } else if(n->children) {
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
            if(n->child) {
               result.push_back(n->child->name);
            }
            if(n->children) {
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
               children(NULL), next(NULL) {}

            prefix_node(char c, prefix_node * cl) : prefix(c), children(cl),
               child(NULL), next(NULL) {}

            prefix_node(char c) : prefix(c), child(NULL), children(NULL),
               next(NULL) {}

            prefix_node * next;
            char prefix;
            prefix_node * children;
            prefix_obj * child;
      };

      prefix_node * map;
};

#endif
