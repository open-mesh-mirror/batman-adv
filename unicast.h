/*
 * Copyright (C) 2010-2012 B.A.T.M.A.N. contributors:
 *
 * Andreas Langer
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

#ifndef _NET_BATMAN_ADV_UNICAST_H_
#define _NET_BATMAN_ADV_UNICAST_H_

#include "packet.h"

#define FRAG_TIMEOUT 10000	/* purge frag list entries after time in ms */
#define FRAG_BUFFER_SIZE 6	/* number of list elements in buffer */

int batadv_frag_reassemble_skb(struct sk_buff *skb, struct bat_priv *bat_priv,
			       struct sk_buff **new_skb);
void batadv_frag_list_free(struct list_head *head);
int batadv_frag_send_skb(struct sk_buff *skb, struct bat_priv *bat_priv,
			 struct hard_iface *hard_iface,
			 const uint8_t dstaddr[]);
bool batadv_prepare_unicast_4addr_packet(struct bat_priv *bat_priv,
					 struct sk_buff *skb,
					 struct orig_node *orig_node,
					 int packet_subtype);
int batadv_unicast_generic_send_skb(struct sk_buff *skb,
				    struct bat_priv *bat_priv,
				    int packet_type, int packet_subtype);

static inline int unicast_send_skb(struct sk_buff *skb,
				   struct bat_priv *bat_priv)
{
	return batadv_unicast_generic_send_skb(skb, bat_priv, BAT_UNICAST, 0);
}

static inline int unicast_4addr_send_skb(struct sk_buff *skb,
					 struct bat_priv *bat_priv,
					 int packet_subtype)
{
	return batadv_unicast_generic_send_skb(skb, bat_priv, BAT_UNICAST_4ADDR,
					       packet_subtype);
}

static inline int frag_can_reassemble(const struct sk_buff *skb, int mtu)
{
	const struct unicast_frag_packet *unicast_packet;
	int uneven_correction = 0;
	unsigned int merged_size;

	unicast_packet = (struct unicast_frag_packet *)skb->data;

	if (unicast_packet->flags & UNI_FRAG_LARGETAIL) {
		if (unicast_packet->flags & UNI_FRAG_HEAD)
			uneven_correction = 1;
		else
			uneven_correction = -1;
	}

	merged_size = (skb->len - sizeof(*unicast_packet)) * 2;
	merged_size += sizeof(struct unicast_packet) + uneven_correction;

	return merged_size <= mtu;
}

#endif /* _NET_BATMAN_ADV_UNICAST_H_ */
