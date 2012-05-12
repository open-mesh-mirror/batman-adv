/* Copyright (C) 2011-2012 B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#ifndef _NET_BATMAN_ADV_ARP_H_
#define _NET_BATMAN_ADV_ARP_H_

#ifdef CONFIG_BATMAN_ADV_DAT

#include "types.h"
#include "originator.h"

#include <linux/if_arp.h>

#define DAT_ADDR_MAX ((dat_addr_t)~(dat_addr_t)0)

#define ARP_HW_SRC(skb, hdr_size) ((uint8_t *)(skb->data + hdr_size) + \
				   ETH_HLEN + sizeof(struct arphdr))
#define ARP_IP_SRC(skb, hdr_size) (*(__be32 *)(ARP_HW_SRC(skb, hdr_size) + \
				   ETH_ALEN))
#define ARP_HW_DST(skb, hdr_size) (ARP_HW_SRC(skb, hdr_size) + ETH_ALEN + 4)
#define ARP_IP_DST(skb, hdr_size) (*(__be32 *)(ARP_HW_SRC(skb, hdr_size) + \
				   ETH_ALEN * 2 + 4))

bool batadv_dat_snoop_outgoing_arp_request(struct bat_priv *bat_priv,
					   struct sk_buff *skb);
bool batadv_dat_snoop_incoming_arp_request(struct bat_priv *bat_priv,
					   struct sk_buff *skb, int hdr_size);
bool batadv_dat_snoop_outgoing_arp_reply(struct bat_priv *bat_priv,
					 struct sk_buff *skb);
bool batadv_dat_snoop_incoming_arp_reply(struct bat_priv *bat_priv,
					 struct sk_buff *skb, int hdr_size);
bool batadv_dat_drop_broadcast_packet(struct bat_priv *bat_priv,
				      struct forw_packet *forw_packet);
void batadv_arp_change_timeout(struct net_device *soft_iface, const char *name);

/* hash function to choose an entry in a hash table of given size.
 * hash algorithm from http://en.wikipedia.org/wiki/Hash_table
 */
static inline uint32_t hash_ipv4(const void *data, uint32_t size)
{
	const unsigned char *key = data;
	uint32_t hash = 0;
	size_t i;

	for (i = 0; i < 4; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % size;
}

static inline void
batadv_dat_init_orig_node_dht_addr(struct orig_node *orig_node)
{
	orig_node->dht_addr = (dat_addr_t)choose_orig(orig_node->orig,
						      DAT_ADDR_MAX);
}

static inline void batadv_dat_init_own_dht_addr(struct bat_priv *bat_priv,
						struct hard_iface *primary_if)
{
	bat_priv->dht_addr = (dat_addr_t)
				choose_orig(primary_if->net_dev->dev_addr,
					    DAT_ADDR_MAX);
}

#else

static inline bool
batadv_dat_snoop_outgoing_arp_request(struct bat_priv *bat_priv,
				      struct sk_buff *skb)
{
	return false;
}

static inline bool
batadv_dat_snoop_incoming_arp_request(struct bat_priv *bat_priv,
				      struct sk_buff *skb, int hdr_size)
{
	return false;
}

static inline bool
batadv_dat_snoop_outgoing_arp_reply(struct bat_priv *bat_priv,
				    struct sk_buff *skb)
{
	return false;
}

static inline bool
batadv_dat_snoop_incoming_arp_reply(struct bat_priv *bat_priv,
				    struct sk_buff *skb, int hdr_size)
{
	return false;
}

static inline bool
batadv_dat_drop_broadcast_packet(struct bat_priv *bat_priv,
				 struct forw_packet *forw_packet)
{
	return false;
}

static inline void
batadv_dat_init_orig_node_dht_addr(struct orig_node *orig_node)
{
}

static inline void batadv_dat_init_own_dht_addr(struct bat_priv *bat_priv,
						struct hard_iface *primary_if)
{
}

static inline void batadv_arp_change_timeout(struct net_device *soft_iface,
					     const char *name)
{
}

#endif /* CONFIG_BATMAN_ADV_DAT */

#endif /* _NET_BATMAN_ADV_ARP_H_ */
