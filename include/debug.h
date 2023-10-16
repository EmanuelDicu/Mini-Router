#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "lib.h"
#include "protocols.h"

void debug_ip_address(uint32_t ip);
void debug_mac_address(uint8_t *mac);

void debug_route_table_entry(struct route_table_entry *rtable_ent);
void debug_arp_entry(struct arp_entry *arp_ent);

void debug_arp_header(struct arp_header *arp_hdr);
void debug_ether_header(struct ether_header *eth_hdr);
void debug_ip_header(struct iphdr *ip_hdr);
void debug_icmp_header(struct icmphdr *icmp_hdr);


#endif /* _DEBUG_H_ */