// SPDX-License-Identifier: BSD-3-Clause

#include "trie.h"

#include <arpa/inet.h>

uint32_t bit_count(uint32_t n)
{
  uint32_t count;

  count = 0;
  while (n) {
    count += n & 1;
    n >>= 1;
  }

  return count;
}

trie new_trie_node()
{
  trie t;

  t = malloc(sizeof(struct trie));
  DIE(t == NULL, "malloc");

  t->rtable_ent = NULL;
  t->children[0] = NULL;
  t->children[1] = NULL;

  return t;
}

trie trie_create(int rtable_len, struct route_table_entry rtable[])
{
  trie t;
  int i;
  
  t = new_trie_node();
  for (i = 0; i < rtable_len; ++i) {
    rtable[i].mask = ntohl(rtable[i].mask);
    rtable[i].next_hop = ntohl(rtable[i].next_hop);
    rtable[i].prefix = ntohl(rtable[i].prefix);
    trie_insert(t, &rtable[i]);
  }

  return t;
}

void trie_insert(trie t, struct route_table_entry* rtable_ent)
{
  int bit;
  int bit_value;
  int rtable_ent_bit_count;

  rtable_ent_bit_count = bit_count(rtable_ent->mask);
  for (bit = 31; bit >= 31 - rtable_ent_bit_count; --bit) {
    bit_value = (rtable_ent->prefix >> bit) & 1;
    if (t->children[bit_value] == NULL) {
      t->children[bit_value] = new_trie_node();
    }
    t = t->children[bit_value];
  }
  t->rtable_ent = rtable_ent;
}

struct route_table_entry* trie_lookup(trie t, uint32_t prefix)
{
  int bit;
  int bit_value;
  struct route_table_entry* rtable_ent;
  int rtable_ent_bit_count;

  rtable_ent = NULL;
  rtable_ent_bit_count = 0;
  for (bit = 31; bit >= 0; --bit) {
    if (t->rtable_ent != NULL &&
        (rtable_ent == NULL || t->rtable_ent->mask > rtable_ent_bit_count)) {
      rtable_ent = t->rtable_ent;
      rtable_ent_bit_count = bit_count(t->rtable_ent->mask);
    }
    bit_value = (prefix >> bit) & 1;
    if (t->children[bit_value] == NULL) {
      break;
    }
    t = t->children[bit_value];
  }

  return rtable_ent;
}