/*
 * Copyright (C) 2011 B.A.T.M.A.N. contributors:
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
 *
 */

#ifndef _NET_BATMAN_ADV_ARP_H_
#define _NET_BATMAN_ADV_ARP_H_

#include "types.h"
#include "originator.h"

#define DAT_ADDR_MAX biggest_unsigned_int(dat_addr_t)

/* hash function to choose an entry in a hash table of given size */
/* hash algorithm from http://en.wikipedia.org/wiki/Hash_table */
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

static inline void dat_init_orig_node_dht_addr(struct orig_node *orig_node)
{
	orig_node->dht_addr = (dat_addr_t)choose_orig(orig_node->orig,
						      DAT_ADDR_MAX);
}

static inline void dat_init_own_dht_addr(struct bat_priv *bat_priv,
					 struct hard_iface *primary_if)
{
	bat_priv->dht_addr = (dat_addr_t)
				choose_orig(primary_if->net_dev->dev_addr,
					    DAT_ADDR_MAX);
}

#endif /* _NET_BATMAN_ADV_ARP_H_ */
