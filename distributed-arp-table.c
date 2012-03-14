/*
 * Copyright (C) 2011-2012 B.A.T.M.A.N. contributors:
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
/* needed to use arp_tbl */
#include <net/arp.h>
#include <linux/inetdevice.h>

#include "main.h"
#include "distributed-arp-table.h"
#include "hard-interface.h"
#include "originator.h"
#include "send.h"
#include "types.h"
#include "translation-table.h"
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
		bat_dbg(DBG_DAT, bat_priv,
			"* encapsulated within a UNICAST packet\n");
		break;
	case BAT_UNICAST_4ADDR:
		bat_dbg(DBG_DAT, bat_priv,
			"* encapsulated within a UNICAST_4ADDR packet (src: %pM)\n",
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
		bat_dbg(DBG_DAT, bat_priv,
			"* encapsulated within a BCAST packet (src: %pM)\n",
			((struct bcast_packet *)unicast_4addr_packet)->orig);
		break;
	default:
		bat_dbg(DBG_DAT, bat_priv,
			"* encapsulated within an unknown packet type (0x%x)\n",
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
		bat_dbg(DBG_DAT, bat_priv,
			"dht_select_candidates() %d: selected %pM addr=%u dist=%u\n",
			select, max_orig_node->orig, max_orig_node->dht_addr,
			max);
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

	bat_dbg(DBG_DAT, bat_priv,
		"dht_select_candidates(): IP=%pI4 hash(IP)=%u\n", &ip_dst,
		ip_key);

	for (select = 0; select < DHT_CANDIDATES_NUM; select++)
		choose_next_candidate(bat_priv, res, select, ip_key, &last_max);

	return res;
}

/* Sends the skb payload passed as argument to the candidates selected for
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

/* Update the neighbour entry corresponding to the IP passed as parameter with
 * the hw address hw. If the neighbour entry doesn't exists, then it will be
 * created */
static void arp_neigh_update(struct bat_priv *bat_priv, uint32_t ip,
			     uint8_t *hw)
{
	struct neighbour *n = NULL;
	struct hard_iface *primary_if = primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	n = __neigh_lookup(&arp_tbl, &ip, primary_if->soft_iface, 1);
	if (!n)
		goto out;

	bat_dbg(DBG_DAT, bat_priv, "Updating neighbour: %pI4 - %pM\n", &ip, hw);

	neigh_update(n, hw, NUD_REACHABLE, NEIGH_UPDATE_F_OVERRIDE);
out:
	if (n && !IS_ERR(n))
		neigh_release(n);
	if (primary_if)
		hardif_free_ref(primary_if);
}

/* Returns arphdr->ar_op if the skb contains a valid ARP packet, otherwise
 * returns 0 */
static uint16_t arp_get_type(struct bat_priv *bat_priv, struct sk_buff *skb,
			     int hdr_size)
{
	struct arphdr *arphdr;
	struct ethhdr *ethhdr;
	uint32_t ip_src, ip_dst;
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
	ip_src = ARP_IP_SRC(skb, hdr_size);
	ip_dst = ARP_IP_DST(skb, hdr_size);
	if (ipv4_is_loopback(ip_src) || ipv4_is_multicast(ip_src) ||
	    ipv4_is_loopback(ip_dst) || ipv4_is_multicast(ip_dst))
		goto out;

	type = ntohs(arphdr->ar_op);
out:
	return type;
}

/* return true if the message has been sent to the dht candidates, false
 * otherwise. In case of true the message has to be enqueued to permit the
 * fallback */
bool dat_snoop_outgoing_arp_request(struct bat_priv *bat_priv,
				    struct sk_buff *skb)
{
	uint16_t type = 0;
	uint32_t ip_dst, ip_src;
	uint8_t *hw_src;
	bool ret = false;
	struct neighbour *n = NULL;
	struct hard_iface *primary_if = NULL;
	struct sk_buff *skb_new;

	type = arp_get_type(bat_priv, skb, 0);
	/* If we get an ARP_REQUEST we have to send the unicast message to the
	 * selected DHT candidates */
	if (type != ARPOP_REQUEST)
		goto out;

	bat_dbg_arp(bat_priv, skb, type, 0, "Parsing outgoing ARP REQUEST");

	ip_src = ARP_IP_SRC(skb, 0);
	hw_src = ARP_HW_SRC(skb, 0);
	ip_dst = ARP_IP_DST(skb, 0);

	primary_if = primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	arp_neigh_update(bat_priv, ip_src, hw_src);

	n = neigh_lookup(&arp_tbl, &ip_dst, primary_if->soft_iface);
	/* check if it is a valid neigh entry */
	if (n && (n->nud_state & NUD_CONNECTED)) {
		skb_new = arp_create(ARPOP_REPLY, ETH_P_ARP, ip_src,
				     primary_if->soft_iface, ip_dst, hw_src,
				     n->ha, hw_src);
		if (!skb_new)
			goto out;

		skb_reset_mac_header(skb_new);
		skb_new->protocol = eth_type_trans(skb_new,
						   primary_if->soft_iface);
		bat_priv->stats.rx_packets++;
		bat_priv->stats.rx_bytes += skb->len + ETH_HLEN;
		primary_if->soft_iface->last_rx = jiffies;

		netif_rx(skb_new);
		bat_dbg(DBG_DAT, bat_priv, "ARP request replied locally\n");
	} else
		/* Send the request on the DHT */
		ret = dht_send_data(bat_priv, skb, ip_dst, BAT_P_DAT_DHT_GET);
out:
	if (n)
		neigh_release(n);
	if (primary_if)
		hardif_free_ref(primary_if);
	return ret;
}

/* This function is meant to be invoked for an ARP request which is coming into
 * the bat0 interfaces from the mesh network. It will check for the needed data
 * into the local table. If found, an ARP reply is sent immediately, otherwise
 * the caller has to deliver the ARP request to the upper layer */
bool dat_snoop_incoming_arp_request(struct bat_priv *bat_priv,
				    struct sk_buff *skb, int hdr_size)
{
	uint16_t type;
	uint32_t ip_src, ip_dst;
	uint8_t *hw_src;
	struct hard_iface *primary_if = NULL;
	struct sk_buff *skb_new;
	struct neighbour *n = NULL;
	bool ret = false;

	type = arp_get_type(bat_priv, skb, hdr_size);
	if (type != ARPOP_REQUEST)
		goto out;

	hw_src = ARP_HW_SRC(skb, hdr_size);
	ip_src = ARP_IP_SRC(skb, hdr_size);
	ip_dst = ARP_IP_DST(skb, hdr_size);

	bat_dbg_arp(bat_priv, skb, type, hdr_size,
		    "Parsing incoming ARP REQUEST");

	primary_if = primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	arp_neigh_update(bat_priv, ip_src, hw_src);

	n = neigh_lookup(&arp_tbl, &ip_dst, primary_if->soft_iface);
	/* check if it is a valid neigh entry */
	if (!n || !(n->nud_state & NUD_CONNECTED))
		goto out;

	skb_new = arp_create(ARPOP_REPLY, ETH_P_ARP, ip_src,
			     primary_if->soft_iface, ip_dst, hw_src, n->ha,
			     hw_src);

	if (!skb_new)
		goto out;

	unicast_4addr_send_skb(skb_new, bat_priv, BAT_P_DAT_CACHE_REPLY);

	ret = true;
out:
	if (n)
		neigh_release(n);
	if (primary_if)
		hardif_free_ref(primary_if);
	if (ret)
		kfree_skb(skb);
	return ret;
}

/* This function is meant to be invoked on an ARP reply packet going into the
 * soft interface. The related neighbour entry has to be updated and the DHT has
 * to be populated as well */
bool dat_snoop_outgoing_arp_reply(struct bat_priv *bat_priv,
				  struct sk_buff *skb)
{
	uint16_t type;
	uint32_t ip_src, ip_dst;
	uint8_t *hw_src, *hw_dst;
	bool ret = false;

	type = arp_get_type(bat_priv, skb, 0);
	if (type != ARPOP_REPLY)
		goto out;

	bat_dbg_arp(bat_priv, skb, type, 0, "Parsing outgoing ARP REPLY");

	hw_src = ARP_HW_SRC(skb, 0);
	ip_src = ARP_IP_SRC(skb, 0);
	hw_dst = ARP_HW_DST(skb, 0);
	ip_dst = ARP_IP_DST(skb, 0);

	arp_neigh_update(bat_priv, ip_src, hw_src);
	arp_neigh_update(bat_priv, ip_dst, hw_dst);

	/* Send the ARP reply to the candidates for both the IP addresses we
	 * fetched from the ARP reply */
	dht_send_data(bat_priv, skb, ip_src, BAT_P_DAT_DHT_PUT);
	dht_send_data(bat_priv, skb, ip_dst, BAT_P_DAT_DHT_PUT);
	ret = true;
out:
	return ret;
}

/* This function has to be invoked on an ARP reply coming into the soft
 * interface from the mesh network. The local table has to be updated */
bool dat_snoop_incoming_arp_reply(struct bat_priv *bat_priv,
				  struct sk_buff *skb, int hdr_size)
{
	uint16_t type;
	uint32_t ip_src, ip_dst;
	uint8_t *hw_src, *hw_dst;
	bool ret = false;

	type = arp_get_type(bat_priv, skb, hdr_size);
	if (type != ARPOP_REPLY)
		goto out;

	bat_dbg_arp(bat_priv, skb, type, hdr_size,
		    "Parsing incoming ARP REPLY");

	hw_src = ARP_HW_SRC(skb, hdr_size);
	ip_src = ARP_IP_SRC(skb, hdr_size);
	hw_dst = ARP_HW_DST(skb, hdr_size);
	ip_dst = ARP_IP_DST(skb, hdr_size);

	/* Update our internal cache with both the IP addresses we fetched from
	 * the ARP reply */
	arp_neigh_update(bat_priv, ip_src, hw_src);
	arp_neigh_update(bat_priv, ip_dst, hw_dst);

	/* if this REPLY is directed to a client of mine, let's deliver the
	 * packet to the interface */
	ret = !is_my_client(bat_priv, hw_dst);
out:
	/* if ret == false packet has to be delivered to the interface */
	return ret;
}

bool dat_drop_broadcast_packet(struct bat_priv *bat_priv,
			       struct forw_packet *forw_packet)
{
	struct neighbour *n;

	/* If this packet is an ARP_REQUEST and we already have the information
	 * that it is going to ask, we can drop the packet */
	if (!forw_packet->num_packets &&
	    (ARPOP_REQUEST == arp_get_type(bat_priv, forw_packet->skb,
					   sizeof(struct bcast_packet)))) {
		n = neigh_lookup(&arp_tbl,
				 &ARP_IP_DST(forw_packet->skb,
					     sizeof(struct bcast_packet)),
				 forw_packet->if_incoming->soft_iface);
		/* check if we already know this neigh */
		if (n && (n->nud_state & NUD_CONNECTED)) {
			bat_dbg(DBG_DAT, bat_priv,
				"ARP Request for %pI4: fallback prevented\n",
				&ARP_IP_DST(forw_packet->skb,
					    sizeof(struct bcast_packet)));
			return true;
		}

		bat_dbg(DBG_DAT, bat_priv, "ARP Request for %pI4: fallback\n",
			&ARP_IP_DST(forw_packet->skb,
				    sizeof(struct bcast_packet)));
	}
	return false;
}

void arp_change_timeout(struct net_device *soft_iface, const char *name)
{
	struct in_device *in_dev = in_dev_get(soft_iface);
	if (!in_dev) {
		pr_err("Unable to set ARP parameters for the batman interface '%s'\n",
		       name);
		return;
	}

	/* Introduce a delay in the ARP state-machine transactions. Entries
	 * will be kept in the ARP table for the default time multiplied by 4 */
	in_dev->arp_parms->base_reachable_time *= ARP_TIMEOUT_FACTOR;
	in_dev->arp_parms->gc_staletime *= ARP_TIMEOUT_FACTOR;
	in_dev->arp_parms->reachable_time *= ARP_TIMEOUT_FACTOR;

	in_dev_put(in_dev);
}
