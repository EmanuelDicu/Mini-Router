#include "queue.h"
#include "lib.h"
#include "trie.h"
#include "protocols.h"
#include "list.h"
#include "headers.h"

struct route_table_entry rtable[RTABLE_LEN];

uint32_t get_ip_from_string(char *ip)
{
  uint32_t ip_addr = 0;
  char *token;
  token = strtok(ip, ".");
  ip_addr = atoi(token);
  for (int i = 0; i < 3; i++) {
    token = strtok(NULL, ".");
    ip_addr = ip_addr << 8;
    ip_addr += atoi(token);
  }
  return ip_addr;
}

int process_target(uint32_t ip)
{
  int i;
  for (i = 0; i < ROUTER_NUM_INTERFACES; i++) {
    if (ip == get_ip_from_string(get_interface_ip(i)))
      return 1;
  }
  return 0;
}

int main(int argc, char* argv[])
{
	char buf[MAX_PACKET_LEN];
  int i, q_len, q_len_r, rtable_len;
  char *rtable_path, *packet;

  queue q; trie t;
  struct ether_header *eth_hdr, eth_hdr_r;
  struct iphdr *ip_hdr, ip_hdr_r;
  struct icmphdr *icmp_hdr, icmp_hdr_r;
  struct arp_header *arp_hdr, arp_hdr_r;

  uint8_t icmp_type;
  uint32_t ip_saddr, ip_daddr;
  uint32_t aux_val;

  struct arp_entry* arp_ent;
  struct route_table_entry* rtable_ent;
  list arp_cache, arp_cache_r;

	// Do not modify this line
	init(argc - 2, argv + 2);

  /* create arp cache */
  arp_cache = NULL;

  /* create packet queue */
  q = queue_create();
  q_len = 0;

  /* read rtable */
  rtable_path = argv[1];
  rtable_len = read_rtable(rtable_path, rtable);
  DIE(rtable_len <= 0, "read_rtable");

  /* create route_table_entry trie and populate it */
  t = trie_create(rtable_len, rtable);

	while (1) {

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");

		eth_hdr = (struct ether_header *) buf;
		/* Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when sending a packet on the link, */

    switch (ntohs(eth_hdr->ether_type)) {
    case ETHERTYPE_IP:
      ip_hdr = (struct iphdr *) (buf + sizeof(*eth_hdr));
      icmp_hdr = (struct icmphdr *) (buf + sizeof(*eth_hdr) + sizeof(*ip_hdr));

      icmp_type = ICMP_ECHO;
      if (process_target(ntohl(ip_hdr->daddr)) && icmp_hdr->type == ICMP_ECHO) {
        icmp_type = ICMP_ECHOREPLY;
      } else 
      if (ip_hdr->ttl <= 1) {
        icmp_type = ICMP_TIME_EXCEEDED;
      } else {
        if (checksum((uint16_t *) ip_hdr, sizeof(*ip_hdr)) != 0) {
          continue;
        }
        ip_hdr->ttl -= 1;
        ip_hdr->check = 0;
        ip_hdr->check = htons(checksum((uint16_t *) ip_hdr, sizeof(*ip_hdr)));

        rtable_ent = trie_lookup(t, htonl(ip_hdr->daddr));
        if (rtable_ent == NULL) {
          icmp_type = ICMP_DEST_UNREACH;
        }
      }

      if (icmp_type != ICMP_ECHO) {
        memcpy(&eth_hdr_r, eth_hdr, sizeof(eth_hdr_r));
        memcpy(&ip_hdr_r, ip_hdr, sizeof(ip_hdr_r));
        memcpy(&icmp_hdr_r, icmp_hdr, sizeof(icmp_hdr_r));

        memcpy(&eth_hdr_r.ether_dhost, &eth_hdr->ether_shost, 6);
        memcpy(&eth_hdr_r.ether_shost, &eth_hdr->ether_dhost, 6);
        eth_hdr_r.ether_type = htons(ETHERTYPE_IP);

        ip_hdr_r.version = 4;
        ip_hdr_r.ihl = 5;
        ip_hdr_r.tos = 0;
        ip_hdr_r.tot_len = htons(sizeof(ip_hdr_r) + sizeof(icmp_hdr_r));
        ip_hdr_r.id = htons(1);
        ip_hdr_r.frag_off = 0;
        ip_hdr_r.ttl = 64;
        ip_hdr_r.protocol = IPPROTO_ICMP;
        ip_hdr_r.saddr = ip_hdr->daddr;
        ip_hdr_r.daddr = ip_hdr->saddr;
        ip_hdr_r.check = 0;
        ip_hdr_r.check = checksum((uint16_t *) &ip_hdr_r, sizeof(ip_hdr_r));

        icmp_hdr_r.type = icmp_type;
        icmp_hdr_r.code = 0;
        icmp_hdr_r.un.echo.id = icmp_hdr->un.echo.id;
        icmp_hdr_r.un.echo.sequence = icmp_hdr->un.echo.sequence;
        icmp_hdr_r.checksum = 0;
        icmp_hdr_r.checksum = checksum((uint16_t *) &icmp_hdr_r, sizeof(icmp_hdr_r));

        memcpy(buf, &eth_hdr_r, sizeof(eth_hdr_r));
        memcpy(buf + sizeof(eth_hdr_r), &ip_hdr_r, sizeof(ip_hdr_r));
        memcpy(buf + sizeof(eth_hdr_r) + sizeof(ip_hdr_r), &icmp_hdr_r, sizeof(icmp_hdr_r));

        send_to_link(interface, buf, sizeof(eth_hdr_r) + sizeof(ip_hdr_r) + sizeof(icmp_hdr_r));
        continue;
      } 

      arp_ent = NULL;
      for (arp_cache_r = arp_cache; arp_cache_r != NULL; arp_cache_r = arp_cache_r->next) {
        if (((struct arp_entry *) (arp_cache_r->element))->ip == htonl(rtable_ent->next_hop)) {
          arp_ent = (struct arp_entry *) (arp_cache_r->element);
          break;
        }
      }

      if (arp_ent == NULL) {
        packet = malloc(len * sizeof(*packet));
        memcpy(packet, buf, len);
        queue_enq(q, packet);
        ++q_len;

        DIE(rtable_ent == NULL, "rtable_ent");
        ip_saddr = htonl(get_ip_from_string(get_interface_ip(rtable_ent->interface)));
        ip_daddr = htonl(rtable_ent->next_hop);

        memset(eth_hdr_r.ether_dhost, 0xff, 6);
        get_interface_mac(rtable_ent->interface, eth_hdr_r.ether_shost);
        eth_hdr_r.ether_type = htons(ETHERTYPE_ARP);

        arp_hdr_r.htype = htons(1);
        arp_hdr_r.ptype = htons(ETHERTYPE_IP);
        arp_hdr_r.hlen = 6;
        arp_hdr_r.plen = 4;
        arp_hdr_r.op = htons(ARPOP_REQUEST);
        memcpy(arp_hdr_r.sha, eth_hdr_r.ether_shost, 6);
        arp_hdr_r.spa = ip_saddr;
        memcpy(arp_hdr_r.tha, eth_hdr_r.ether_dhost, 6);
        arp_hdr_r.tpa = ip_daddr;

        memcpy(buf, &eth_hdr_r, sizeof(eth_hdr_r));
        memcpy(buf + sizeof(eth_hdr_r), &arp_hdr_r, sizeof(arp_hdr_r));
        send_to_link(rtable_ent->interface, buf, sizeof(eth_hdr_r) + sizeof(arp_hdr_r)); // ok
      } else {
        memcpy(&eth_hdr_r, eth_hdr, sizeof(eth_hdr_r));
        get_interface_mac(rtable_ent->interface, eth_hdr_r.ether_shost);
        memcpy(eth_hdr_r.ether_dhost, arp_ent->mac, 6);

        memcpy(buf, &eth_hdr_r, sizeof(eth_hdr_r));
        send_to_link(rtable_ent->interface, buf, len);
      }

      break;
    case ETHERTYPE_ARP:
      arp_hdr = (struct arp_header *) (buf + sizeof(*eth_hdr));

      switch (ntohs(arp_hdr->op)) {
      case ARPOP_REQUEST:
        if (process_target(ntohl(arp_hdr->tpa))) {
          memcpy(&eth_hdr_r, eth_hdr, sizeof(eth_hdr_r));
          memcpy(&arp_hdr_r, arp_hdr, sizeof(arp_hdr_r));
         
          memcpy(eth_hdr_r.ether_dhost, eth_hdr->ether_shost, 6);
          get_interface_mac(interface, eth_hdr_r.ether_shost);

          arp_hdr_r.op = htons(ARPOP_REPLY);
          aux_val = arp_hdr_r.spa;
          arp_hdr_r.spa = arp_hdr_r.tpa;
          arp_hdr_r.tpa = aux_val;
          memcpy(arp_hdr_r.sha, eth_hdr_r.ether_shost, 6);
          memcpy(arp_hdr_r.tha, eth_hdr_r.ether_dhost, 6);

          memcpy(buf, &eth_hdr_r, sizeof(eth_hdr_r));
          memcpy(buf + sizeof(eth_hdr_r), &arp_hdr_r, sizeof(arp_hdr_r));
          send_to_link(interface, buf, sizeof(eth_hdr_r) + sizeof(arp_hdr_r));
        }
        break;
      case ARPOP_REPLY:
        arp_ent = (struct arp_entry *) malloc(sizeof(*arp_ent));
        DIE(arp_ent == NULL, "malloc");

        arp_ent->ip = arp_hdr->spa;
        memcpy(arp_ent->mac, arp_hdr->sha, 6);

        arp_cache = cons(arp_ent, arp_cache);
        q_len_r = q_len;

        for (i = 0; i < q_len; ++i) {
          packet = (char *) queue_deq(q);
          memcpy(&eth_hdr_r, packet, sizeof(eth_hdr_r));
          memcpy(&ip_hdr_r, packet + sizeof(eth_hdr_r), sizeof(ip_hdr_r));

          rtable_ent = trie_lookup(t, htonl(ip_hdr_r.daddr));
          DIE(rtable_ent == NULL, "trie_lookup");

          if (rtable_ent->next_hop != htonl(arp_hdr->spa)) {
            queue_enq(q, packet);
          } else {
            memcpy(&eth_hdr_r.ether_dhost, arp_ent->mac, 6);
            get_interface_mac(rtable_ent->interface, eth_hdr_r.ether_shost);

            memcpy(packet, &eth_hdr_r, sizeof(eth_hdr_r));
            memcpy(packet + sizeof(eth_hdr_r), &ip_hdr_r, sizeof(ip_hdr_r));
            send_to_link(rtable_ent->interface, packet, sizeof(eth_hdr_r) + sizeof(ip_hdr_r));
            free(packet);
            --q_len_r;
          }
        }
        q_len = q_len_r;
        break;
      }
      break;
    }
	}

  return 0;
}