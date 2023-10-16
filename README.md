# Router Dataplane Implementation

This project implements a router's dataplane with a Longest Prefix Match routing algorithm. The routing table is implemented using a Trie tree, which allows for fast lookups and insertions.

## Overview

The key helper procedures implemented in this project are as follows:

1. **`trie_insert()`**: This procedure inserts an entry from the routing table into the Trie tree, traversing the tree from the most significant to the least significant bits in the mask, as long as the bits in the mask are set to 1.

2. **`trie_lookup()`**: Given an IP address, this procedure traverses the Trie tree to find the most specific entry, keeping track of more specific entries found in the visited nodes.

3. **`get_ip_from_string()`**: This procedure converts a string representation of an IP address into its `uint32_t` representation.

4. **`process_target()`**: This procedure receives the IP address of a router and determines if there is an interface whose IP address matches the given one.

# Implementation

The router's dataplane implemented in the `router.c` file represents the core of the router's functionality.

### Initialization

The program begins by initializing essential data structures, including the ARP cache and a packet queue. This setup ensures that the router's dataplane can efficiently process incoming packets.

### Routing Table (rtable)

The routing table, typically read from an external file, is used to create and populate a Trie data structure. This routing table is the cornerstone for making routing decisions. The Trie structure provides efficient Longest Prefix Match lookups, helping the router make informed forwarding choices.

### Main Loop

The core functionality of the router's dataplane is executed within the main loop. This loop is where the majority of packet processing and routing decisions take place. Key steps in this loop include:

### Packet Handling

- **Packet Type Detection**: The program diligently examines incoming Ethernet frames to determine their type, whether they are ARP or IP packets.

- **IP Packet Handling**: IP packets, when identified, are processed in the following ways:

  - ICMP Handling: ICMP packets are closely inspected. If the IP packet is an ICMP type, the program responds accordingly, whether it's an ICMP_ECHO_REQUEST or other ICMP packet types such as ICMP_TIME_EXCEEDED or ICMP_DEST_UNREACH.

  - Forwarding: For non-ICMP packets destined for other hosts, the program processes them for forwarding. This involves actions such as decrementing the Time-to-Live (TTL) field and performing routing table lookups to determine the correct path for the packet.

- **ARP Packet Handling**: ARP packets, both REQUEST and REPLY, are handled as follows:

     - ARP REQUEST: If an ARP REQUEST is received and the router is the target, the router sends back an ARP REPLY, facilitating the resolution of MAC addresses.

     - ARP REPLY: For ARP REPLY packets, the program updates the ARP cache, processes any queued packets that can now be forwarded, and forwards them to the appropriate next hop.

### ARP Cache

The router's dataplane maintains an ARP cache, which stores ARP entries. This cache serves a crucial role in efficient packet forwarding by associating IP addresses with their corresponding MAC addresses. The ARP cache is updated whenever an ARP REPLY is received, and it is used to resolve MAC addresses for packets that need to be forwarded.