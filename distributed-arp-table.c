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

#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include "main.h"
#include "distributed-arp-table.h"
#include "hard-interface.h"
#include "originator.h"
#include "send.h"
#include "types.h"
#include "unicast.h"

#ifdef CONFIG_BATMAN_ADV_DEBUG

static void bat_dbg_arp(struct bat_priv *bat_priv, struct sk_buff *skb,
			uint16_t type, int hdr_size, char *msg) {
	struct unicast_4addr_packet *unicast_4addr_packet;

	if (msg)
		bat_dbg(DBG_DAT, bat_priv, "%s\n", msg);

	bat_dbg(DBG_DAT, bat_priv, "ARP MSG = [src: %pM-%pI4 dst: %pM-%pI4]\n",
		ARP_HW_SRC(skb, hdr_size), &ARP_IP_SRC(skb, hdr_size),
		ARP_HW_DST(skb, hdr_size), &ARP_IP_DST(skb, hdr_size));

	if (hdr_size == 0)
		return;

	/* if the AP packet is encapsulated in a batman packet, let's print some
	 * debug messages */
	unicast_4addr_packet = (struct unicast_4addr_packet *)skb->data;

	switch (unicast_4addr_packet->u.header.packet_type) {
	case BAT_UNICAST:
		bat_dbg(DBG_DAT, bat_priv, "* encapsulated within a UNICAST "
			"packet\n");
		break;
	case BAT_UNICAST_4ADDR:
		bat_dbg(DBG_DAT, bat_priv, "* encapsulated within a "
			"UNICAST_4ADDR packet (src: %pM)\n",
			unicast_4addr_packet->src);
		switch (unicast_4addr_packet->subtype) {
		case BAT_P_DAT_DHT_PUT:
			bat_dbg(DBG_DAT, bat_priv, "* type: DAT_DHT_PUT\n");
			break;
		case BAT_P_DAT_DHT_GET:
			bat_dbg(DBG_DAT, bat_priv, "* type: DAT_DHT_GET\n");
			break;
		case BAT_P_DAT_CACHE_REPLY:
			bat_dbg(DBG_DAT, bat_priv, "* type: DAT_CACHE_REPLY\n");
			break;
		case BAT_P_DATA:
			bat_dbg(DBG_DAT, bat_priv, "* type: DATA\n");
			break;
		default:
			bat_dbg(DBG_DAT, bat_priv, "* type: Unknown!\n");
		}
		break;
	case BAT_BCAST:
		bat_dbg(DBG_DAT, bat_priv, "* encapsulated within a BCAST "
			"packet (src: %pM)\n",
			((struct bcast_packet *)unicast_4addr_packet)->orig);
		break;
	default:
		bat_dbg(DBG_DAT, bat_priv, "* encapsulated within an unknown "
			"packet type (0x%x)\n",
			unicast_4addr_packet->u.header.packet_type);
	}
}

#else

#define bat_dbg_arp(...)

#endif /* CONFIG_BATMAN_ADV_DEBUG */

static bool is_orig_node_eligible(struct dht_candidate *res, int select,
				  dat_addr_t tmp_max, dat_addr_t max,
				  dat_addr_t last_max,
				  struct orig_node *candidate,
				  struct orig_node *max_orig_node)
{
	bool ret = false;
	int j;

	/* Check if we have already selected this neighbour... */
	for (j = 0; j < select; j++)
		if (res[j].orig_node == candidate)
			break;
	/* ..and possibly skip it */
	if (j < select)
		goto out;
	/* sanity check: has it already been selected? This should not happen */
	if (tmp_max > last_max)
		goto out;
	/* check if during this iteration we have already found an originator
	 * with a closer dht address */
	if (tmp_max < max)
		goto out;
	/* this is an hash collision with the temporary selected node. Choose
	 * the one with the lowest address */
	if ((tmp_max == max) &&
	    (compare_eth(candidate->orig, max_orig_node->orig) > 0))
		goto out;

	ret = true;
out:
	return ret;
}

/* selects the next candidate by populating cands[select] and modifies last_max
 * accordingly */
static void choose_next_candidate(struct bat_priv *bat_priv,
				  struct dht_candidate *cands, int select,
				  dat_addr_t ip_key, dat_addr_t *last_max)
{
	dat_addr_t max = 0, tmp_max = 0;
	struct orig_node *orig_node, *max_orig_node = NULL;
	struct hashtable_t *hash = bat_priv->orig_hash;
	struct hlist_node *node;
	struct hlist_head *head;
	int i;

	/* if no node is eligible as candidate, we will leave the candidate as
	 * NOT_FOUND */
	cands[select].type = DHT_CANDIDATE_NOT_FOUND;

	/* iterate over the originator list and find the node with closest
	 * dht_address which has not been selected yet */
	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(orig_node, node, head, hash_entry) {
			/* the dht space is a ring and addresses are unsigned */
			tmp_max = DAT_ADDR_MAX - orig_node->dht_addr + ip_key;

			if (!is_orig_node_eligible(cands, select, tmp_max, max,
						   *last_max, orig_node,
						   max_orig_node))
				continue;

			if (!atomic_inc_not_zero(&orig_node->refcount))
				continue;

			max = tmp_max;
			if (max_orig_node)
				orig_node_free_ref(max_orig_node);
			max_orig_node = orig_node;
		}
		rcu_read_unlock();
	}
	if (max_orig_node) {
		cands[select].type = DHT_CANDIDATE_ORIG;
		cands[select].orig_node = max_orig_node;
		bat_dbg(DBG_DAT, bat_priv, "dht_select_candidates() %d: "
			"selected %pM addr=%u dist=%u\n", select,
			max_orig_node->orig, max_orig_node->dht_addr, max);
	}
	*last_max = max;
}

/* Given a key, selects the candidates which the DHT message has to be sent to.
 * An originator O is selected if and only if its DHT_ID value is one of three
 * closest values (from the LEFT, with wrap around if needed) then the hash
 * value of the key. ip_dst is the key.
 *
 * return an array of size DHT_CANDIDATES_NUM */
static struct dht_candidate *dht_select_candidates(struct bat_priv *bat_priv,
						   uint32_t ip_dst)
{
	int select;
	dat_addr_t last_max = DAT_ADDR_MAX, ip_key;
	struct dht_candidate *res;

	if (!bat_priv->orig_hash)
		return NULL;

	res = kmalloc(DHT_CANDIDATES_NUM * sizeof(*res), GFP_ATOMIC);
	if (!res)
		return NULL;

	ip_key = (dat_addr_t)hash_ipv4(&ip_dst, DAT_ADDR_MAX);

	bat_dbg(DBG_DAT, bat_priv, "dht_select_candidates(): IP=%pI4 "
		"hash(IP)=%u\n", &ip_dst, ip_key);

	for (select = 0; select < DHT_CANDIDATES_NUM; select++)
		choose_next_candidate(bat_priv, res, select, ip_key, &last_max);

	return res;
}

/*
 * Sends the skb payload passed as argument to the candidates selected for
 * the data represented by 'ip'. The skb is copied by means of pskb_copy()
 * and is sent as unicast packet to each of the selected candidate.
 *
 * If the packet is successfully sent to at least one candidate, then this
 * function returns true */
static bool dht_send_data(struct bat_priv *bat_priv, struct sk_buff *skb,
			  uint32_t ip, int packet_subtype)
{
	int i;
	bool ret = false;
	struct neigh_node *neigh_node = NULL;
	struct sk_buff *tmp_skb;
	struct dht_candidate *cand = dht_select_candidates(bat_priv, ip);

	if (!cand)
		goto out;

	bat_dbg(DBG_DAT, bat_priv, "DHT_SEND for %pI4\n", &ip);

	for (i = 0; i < DHT_CANDIDATES_NUM; i++) {
		if (cand[i].type == DHT_CANDIDATE_NOT_FOUND)
			continue;

		neigh_node = orig_node_get_router(cand[i].orig_node);
		if (!neigh_node)
			goto free_orig;

		tmp_skb = pskb_copy(skb, GFP_ATOMIC);
		if (!prepare_unicast_4addr_packet(bat_priv, tmp_skb,
						  cand[i].orig_node,
						  packet_subtype)) {
			kfree_skb(tmp_skb);
			goto free_neigh;
		}
		if (send_skb_packet(tmp_skb, neigh_node->if_incoming,
				    neigh_node->addr) == NET_XMIT_SUCCESS)
			/* packet sent to a candidate: we can return true */
			ret = true;
free_neigh:
		neigh_node_free_ref(neigh_node);
free_orig:
		orig_node_free_ref(cand[i].orig_node);
	}

out:
	kfree(cand);
	return ret;
}

/* Returns arphdr->ar_op if the skb contains a valid ARP packet, otherwise
 * returns 0 */
static uint16_t arp_get_type(struct bat_priv *bat_priv, struct sk_buff *skb,
			     int hdr_size)
{
	struct arphdr *arphdr;
	struct ethhdr *ethhdr;
	uint16_t type = 0;

	/* pull the ethernet header */
	if (unlikely(!pskb_may_pull(skb, hdr_size + ETH_HLEN)))
		goto out;

	ethhdr = (struct ethhdr *)(skb->data + hdr_size);

	if (ethhdr->h_proto != htons(ETH_P_ARP))
		goto out;

	/* pull the ARP payload */
	if (unlikely(!pskb_may_pull(skb, hdr_size + ETH_HLEN +
				    arp_hdr_len(skb->dev))))
		goto out;

	arphdr = (struct arphdr *)(skb->data + hdr_size + ETH_HLEN);

	/* Check whether the ARP packet carries a valid
	 * IP information */
	if (arphdr->ar_hrd != htons(ARPHRD_ETHER))
		goto out;

	if (arphdr->ar_pro != htons(ETH_P_IP))
		goto out;

	if (arphdr->ar_hln != ETH_ALEN)
		goto out;

	if (arphdr->ar_pln != 4)
		goto out;

	/* Check for bad reply/request. If the ARP message is not sane, DAT
	 * will simply ignore it */
	if (ipv4_is_loopback(ARP_IP_SRC(skb, hdr_size)) ||
	    ipv4_is_multicast(ARP_IP_SRC(skb, hdr_size)) ||
	    ipv4_is_loopback(ARP_IP_DST(skb, hdr_size)) ||
	    ipv4_is_multicast(ARP_IP_DST(skb, hdr_size)))
		goto out;

	type = ntohs(arphdr->ar_op);
out:
	return type;
}
