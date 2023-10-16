#ifndef _TRIE_H_
#define _TRIE_H_

#include "lib.h"

struct trie 
{
  struct route_table_entry *rtable_ent;
  struct trie* children[2];
};
typedef struct trie* trie;

/* create an empty trie */
trie trie_create(int rtable_len, struct route_table_entry rtable[]);

/* insert a route_table_entry into the trie */
void trie_insert(trie t, struct route_table_entry* rtable_ent);

/* lookup the best route_table_entry for a given prefix */
struct route_table_entry* trie_lookup(trie t, uint32_t ip);

#endif /* _TRIE_H_ */