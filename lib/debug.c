#include "debug.h"
#include <stdio.h>

void debug_ip_address(uint32_t ip) {
  printf("%d.%d.%d.%d\n", 
    (ip >> 24) & 0xff, 
    (ip >> 16) & 0xff, 
    (ip >> 8) & 0xff, 
    ip & 0xff);
}

void debug_mac_address(uint8_t *mac) {
  printf("%x:%x:%x:%x:%x:%x\n", 
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void debug_route_table_entry(struct route_table_entry *rtable_ent)
{
  printf("Route table entry:\n");
  if (rtable_ent == NULL) {
    printf("NULL\n");
    return;
  }
  printf("prefix: "); debug_ip_address(rtable_ent->prefix);
  printf("mask: "); debug_ip_address(rtable_ent->mask);
  printf("next_hop: "); debug_ip_address(rtable_ent->next_hop);
  printf("interface: %d\n", rtable_ent->interface);
}

void debug_arp_entry(struct arp_entry *arp_ent)
{
  printf("ARP entry:\n");
  if (arp_ent == NULL) {
    printf("NULL\n");
    return;
  }
  printf("ip: "); debug_ip_address(arp_ent->ip);
  printf("mac: "); debug_mac_address(arp_ent->mac);
}

void debug_arp_header(struct arp_header *arp_hdr)
{
  printf("ARP header:\n");
  if (arp_hdr == NULL) {
    printf("NULL\n");
    return;
  }
  printf("htype: %x\n", arp_hdr->htype);
  printf("ptype: %x\n", arp_hdr->ptype);
  printf("hlen: %x\n", arp_hdr->hlen);
  printf("plen: %x\n", arp_hdr->plen);
  printf("op: %x\n", arp_hdr->op);
  printf("sha: "); debug_mac_address(arp_hdr->sha);
  printf("spa: "); debug_ip_address(arp_hdr->spa);
  printf("tha: "); debug_mac_address(arp_hdr->tha);
  printf("tpa: "); debug_ip_address(arp_hdr->tpa);
}

void debug_ether_header(struct ether_header *eth_hdr)
{
  printf("Ethernet header:\n");
  if (eth_hdr == NULL) {
    printf("NULL\n");
    return;
  }
  printf("destination: "); debug_mac_address(eth_hdr->ether_dhost);
  printf("source: "); debug_mac_address(eth_hdr->ether_shost);
  printf("type: %x\n", eth_hdr->ether_type);
}

void debug_ip_header(struct iphdr *ip_hdr)
{
  printf("IP header:\n");
  if (ip_hdr == NULL) {
    printf("NULL\n");
    return;
  }
  printf("version: %x\n", ip_hdr->version);
  printf("ihl: %x\n", ip_hdr->ihl);
  printf("tos: %x\n", ip_hdr->tos);
  printf("tot_len: %x\n", ip_hdr->tot_len);
  printf("id: %x\n", ip_hdr->id);
  printf("frag_off: %x\n", ip_hdr->frag_off);
  printf("ttl: %x\n", ip_hdr->ttl);
  printf("protocol: %x\n", ip_hdr->protocol);
  printf("check: %x\n", ip_hdr->check);
  printf("saddr: "); debug_ip_address(ip_hdr->saddr);
  printf("daddr: "); debug_ip_address(ip_hdr->daddr);
}

void debug_icmp_header(struct icmphdr *icmp_hdr)
{
  printf("ICMP header:\n");
  if (icmp_hdr == NULL) {
    printf("NULL\n");
    return;
  }
  printf("type: %x\n", icmp_hdr->type);
  printf("code: %x\n", icmp_hdr->code);
  printf("checksum: %x\n", icmp_hdr->checksum);
  printf("un.echo.id: %x\n", icmp_hdr->un.echo.id);
  printf("un.echo.sequence: %x\n", icmp_hdr->un.echo.sequence);
}