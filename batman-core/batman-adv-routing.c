/*
 * Copyright (C) 2007 B.A.T.M.A.N. contributors:
 * Marek Lindner
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





#include "batman-adv-main.h"
#include "batman-adv-routing.h"
#include "batman-adv-log.h"
#include "batman-adv-send.h"
#include "types.h"
#include "hash.h"
#include "ring_buffer.h"



DECLARE_WAIT_QUEUE_HEAD(thread_wait);

atomic_t data_ready_cond;
atomic_t exit_cond;



struct neigh_node *create_neighbor(struct orig_node *orig_node, struct orig_node *orig_neigh_node, uint8_t *neigh, struct batman_if *if_incoming)
{
	struct neigh_node *neigh_node;

	debug_log(LOG_TYPE_ROUTING, "Creating new last-hop neighbour of originator\n");

	neigh_node = kmalloc(sizeof(struct neigh_node), GFP_KERNEL);
	memset(neigh_node, 0, sizeof(struct neigh_node));
	INIT_LIST_HEAD(&neigh_node->list);

	memcpy(neigh_node->addr, neigh, ETH_ALEN);
	neigh_node->orig_node = orig_neigh_node;
	neigh_node->if_incoming = if_incoming;

	list_add_tail(&neigh_node->list, &orig_node->neigh_list);
	return neigh_node;
}

void free_orig_node(void *data)
{
	struct list_head *list_pos, *list_pos_tmp;
	struct neigh_node *neigh_node;
	struct orig_node *orig_node = (struct orig_node *)data;

	/* for all neighbours towards this originator ... */
	list_for_each_safe(list_pos, list_pos_tmp, &orig_node->neigh_list) {
		neigh_node = list_entry(list_pos, struct neigh_node, list);

		list_del(list_pos);
		kfree(neigh_node);
	}

	kfree(orig_node->bcast_own);
	kfree(orig_node->bcast_own_sum);
	kfree(orig_node);
}

/* this function finds or creates an originator entry for the given address if it does not exits */
struct orig_node *get_orig_node(uint8_t *addr)
{
	struct orig_node *orig_node;
	struct hashtable_t *swaphash;
	static char orig_str[ETH_STR_LEN];

	orig_node = ((struct orig_node *)hash_find(orig_hash, addr));

	if (orig_node != NULL)
		return orig_node;

	addr_to_string(orig_str, addr);
	debug_log(LOG_TYPE_ROUTING, "Creating new originator: %s \n", orig_str);

	orig_node = kmalloc(sizeof(struct orig_node), GFP_KERNEL);
	memset(orig_node, 0, sizeof(struct orig_node));
	INIT_LIST_HEAD(&orig_node->neigh_list);

	memcpy(orig_node->orig, addr, ETH_ALEN);
	orig_node->router = NULL;
	orig_node->batman_if = NULL;

	orig_node->bcast_own = kmalloc(num_ifs * sizeof(TYPE_OF_WORD) * NUM_WORDS, GFP_KERNEL);
	memset(orig_node->bcast_own, 0, num_ifs * sizeof(TYPE_OF_WORD) * NUM_WORDS);

	orig_node->bcast_own_sum = kmalloc(num_ifs * sizeof(uint8_t), GFP_KERNEL);
	memset(orig_node->bcast_own_sum, 0, num_ifs * sizeof(uint8_t));

	hash_add(orig_hash, orig_node);

	if (orig_hash->elements * 4 > orig_hash->size) {
		swaphash = hash_resize(orig_hash, orig_hash->size * 2);

		if (swaphash == NULL)
			debug_log(LOG_TYPE_CRIT, "Couldn't resize hash table \n");
		else
			orig_hash = swaphash;
	}

	return orig_node;
}

int isBidirectionalNeigh(struct orig_node *orig_node, struct orig_node *orig_neigh_node, struct batman_packet *batman_packet, struct batman_if *if_incoming)
{
	struct list_head *list_pos;
	struct neigh_node *neigh_node = NULL, *tmp_neigh_node = NULL;
	static char orig_str[ETH_STR_LEN], neigh_str[ETH_STR_LEN];
	uint8_t total_count;

	addr_to_string(orig_str, orig_node->orig);
	addr_to_string(neigh_str, orig_neigh_node->orig);

	if (orig_node == orig_neigh_node) {
		list_for_each(list_pos, &orig_node->neigh_list) {
			tmp_neigh_node = list_entry(list_pos, struct neigh_node, list);

			if (compare_orig(tmp_neigh_node->addr, orig_neigh_node->orig) && (tmp_neigh_node->if_incoming == if_incoming))
				neigh_node = tmp_neigh_node;
		}

		if (neigh_node == NULL)
			neigh_node = create_neighbor(orig_node, orig_neigh_node, orig_neigh_node->orig, if_incoming);

		neigh_node->last_valid = jiffies;
	} else {
		/* find packet count of corresponding one hop neighbor */
		list_for_each(list_pos, &orig_neigh_node->neigh_list) {
			tmp_neigh_node = list_entry(list_pos, struct neigh_node, list);

			if (compare_orig(tmp_neigh_node->addr, orig_neigh_node->orig) && (tmp_neigh_node->if_incoming == if_incoming))
				neigh_node = tmp_neigh_node;
		}

		if ( neigh_node == NULL )
			neigh_node = create_neighbor(orig_neigh_node, orig_neigh_node, orig_neigh_node->orig, if_incoming);
	}

	orig_node->last_valid = jiffies;

	/* pay attention to not get a value bigger than 100 % */
	total_count = (orig_neigh_node->bcast_own_sum[if_incoming->if_num] > neigh_node->real_packet_count ? neigh_node->real_packet_count : orig_neigh_node->bcast_own_sum[if_incoming->if_num]);

	/* if we have too few packets (too less data) we set tq_own to zero */
	/* if we receive too few packets it is not considered bidirectional */
	if ((total_count < TQ_LOCAL_BIDRECT_SEND_MINIMUM) || (neigh_node->real_packet_count < TQ_LOCAL_BIDRECT_RECV_MINIMUM))
		orig_neigh_node->tq_own = 0;
	else
		/* neigh_node->real_packet_count is never zero as we only purge old information when getting new information */
		orig_neigh_node->tq_own = (TQ_MAX_VALUE * total_count) / neigh_node->real_packet_count;

	/* 1 - ((1-x)**2), normalized to TQ_MAX_VALUE */
	/* this does affect the nearly-symmetric links only a little,
	* but punishes asymetric links more. */
	/* this will give a value between 0 and TQ_MAX_VALUE */
	orig_neigh_node->tq_asym_penality = TQ_MAX_VALUE - (TQ_MAX_VALUE * (TQ_LOCAL_WINDOW_SIZE - neigh_node->real_packet_count) * (TQ_LOCAL_WINDOW_SIZE - neigh_node->real_packet_count))
										/ (TQ_LOCAL_WINDOW_SIZE * TQ_LOCAL_WINDOW_SIZE);

	batman_packet->tq = ((batman_packet->tq * orig_neigh_node->tq_own * orig_neigh_node->tq_asym_penality) / (TQ_MAX_VALUE *  TQ_MAX_VALUE));

	debug_log(LOG_TYPE_ROUTING, "bidirectional: orig = %-15s neigh = %-15s => own_bcast = %2i, real recv = %2i, local tq: %3i, asym_penality: %3i, total tq: %3i \n",
		      orig_str, neigh_str, total_count, neigh_node->real_packet_count, orig_neigh_node->tq_own, orig_neigh_node->tq_asym_penality, batman_packet->tq);

	/* if link has the minimum required transmission quality consider it bidirectional */
	if (batman_packet->tq >= TQ_TOTAL_BIDRECT_LIMIT)
		return 1;

	return 0;
}

void update_orig(struct orig_node *orig_node, struct ethhdr *ethhdr, struct batman_packet *batman_packet, struct batman_if *if_incoming, unsigned char *hna_buff, int hna_buff_len, char is_duplicate)
{
	struct list_head *list_pos;
	struct neigh_node *neigh_node = NULL, *tmp_neigh_node = NULL, *best_neigh_node = NULL;
	char max_tq = 0, max_bcast_own = 0;

	debug_log(LOG_TYPE_ROUTING, "update_originator(): Searching and updating originator entry of received packet,  \n");

	list_for_each(list_pos, &orig_node->neigh_list) {
		tmp_neigh_node = list_entry(list_pos, struct neigh_node, list);

		if (compare_orig(tmp_neigh_node->addr, ethhdr->h_source) && (tmp_neigh_node->if_incoming == if_incoming)) {
			neigh_node = tmp_neigh_node;
		} else {

			if (!is_duplicate) {
				ring_buffer_set(tmp_neigh_node->tq_recv, &tmp_neigh_node->tq_index, 0);
				tmp_neigh_node->tq_avg = ring_buffer_avg(tmp_neigh_node->tq_recv);
			}

			/* if we got have a better tq value via this neighbour or same tq value if it is currently our best neighbour (to avoid route flipping) */
			if ((tmp_neigh_node->tq_avg > max_tq) || ((tmp_neigh_node->tq_avg == max_tq) && (tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own)) || (( orig_node->router == tmp_neigh_node) && (tmp_neigh_node->tq_avg == max_tq))) {

				max_tq = tmp_neigh_node->tq_avg;
				max_bcast_own = tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num];
				best_neigh_node = tmp_neigh_node;
			}
		}
	}

	if (neigh_node == NULL)
		neigh_node = create_neighbor(orig_node, get_orig_node(ethhdr->h_source), ethhdr->h_source, if_incoming);
	else
		debug_log(LOG_TYPE_ROUTING, "Updating existing last-hop neighbour of originator\n");

	neigh_node->last_valid = jiffies;

	ring_buffer_set(neigh_node->tq_recv, &neigh_node->tq_index, batman_packet->tq);
	neigh_node->tq_avg = ring_buffer_avg(neigh_node->tq_recv);

	if (!is_duplicate) {
		orig_node->last_ttl = batman_packet->ttl;
		neigh_node->last_ttl = batman_packet->ttl;
	}

	if ((neigh_node->tq_avg > max_tq) || ((neigh_node->tq_avg == max_tq) && (neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own)) || ((orig_node->router == neigh_node) && (neigh_node->tq_avg == max_tq))) {

		max_tq = neigh_node->tq_avg;
		max_bcast_own = neigh_node->orig_node->bcast_own_sum[if_incoming->if_num];
		best_neigh_node = neigh_node;

	}

	/* TODO: check for changed hna announcements */
}

char count_real_packets(struct ethhdr *ethhdr, struct batman_packet *batman_packet, struct batman_if *if_incoming)
{
	struct list_head *list_pos;
	struct orig_node *orig_node;
	struct neigh_node *tmp_neigh_node;
	char is_duplicate = 0;


	orig_node = get_orig_node(batman_packet->orig);

	list_for_each(list_pos, &orig_node->neigh_list) {
		tmp_neigh_node = list_entry(list_pos, struct neigh_node, list);

		if (!is_duplicate)
			is_duplicate = get_bit_status(tmp_neigh_node->real_bits, orig_node->last_real_seqno, batman_packet->seqno);

		if (compare_orig(tmp_neigh_node->addr, ethhdr->h_source) && (tmp_neigh_node->if_incoming == if_incoming))
			bit_get_packet(tmp_neigh_node->real_bits, batman_packet->seqno - orig_node->last_real_seqno, 1);
		else
			bit_get_packet(tmp_neigh_node->real_bits, batman_packet->seqno - orig_node->last_real_seqno, 0);

		tmp_neigh_node->real_packet_count = bit_packet_count(tmp_neigh_node->real_bits);
	}

	if (!is_duplicate) {
		debug_log(LOG_TYPE_ROUTING, "updating last_seqno: old %d, new %d \n", orig_node->last_real_seqno, batman_packet->seqno);
		orig_node->last_real_seqno = batman_packet->seqno;
	}

	return is_duplicate;
}

void receive_bat_packet(struct ethhdr *ethhdr, struct batman_packet *batman_packet, unsigned char *hna_buff, int hna_buff_len, struct batman_if *if_incoming)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	struct orig_node *orig_neigh_node, *orig_node;
	static char orig_str[ETH_STR_LEN], old_orig_str[ETH_STR_LEN], neigh_str[ETH_STR_LEN];
	char has_unidirectional_flag, has_directlink_flag;
	int is_my_addr = 0, is_my_orig = 0, is_my_oldorig = 0, is_broadcast = 0, is_bidirectional, is_single_hop_neigh, is_duplicate;
	uint16_t if_incoming_seqno;

	/* could be changed by send_own_packet() */
	spin_lock(&if_incoming->seqno_lock);
	if_incoming_seqno = if_incoming->seqno;
	spin_unlock(&if_incoming->seqno_lock);

	addr_to_string(orig_str, batman_packet->orig);
	addr_to_string(old_orig_str, batman_packet->old_orig);
	addr_to_string(neigh_str, ethhdr->h_source);

	has_unidirectional_flag = (batman_packet->flags & UNIDIRECTIONAL ? 1 : 0);
	has_directlink_flag = (batman_packet->flags & DIRECTLINK ? 1 : 0);

	is_single_hop_neigh = (compare_orig(ethhdr->h_source, batman_packet->orig) ? 1 : 0);

	debug_log(LOG_TYPE_ROUTING, "Received BATMAN packet via NB: %s, IF: %s [%s] (from OG: %s, via old OG: %s, seqno %d, tq %d, TTL %d, V %d, UDF %d, IDF %d) \n", neigh_str, if_incoming->net_dev->name, if_incoming->addr_str, orig_str, old_orig_str, batman_packet->seqno, batman_packet->tq, batman_packet->ttl, batman_packet->version, has_unidirectional_flag, has_directlink_flag);

	list_for_each(list_pos, &if_list) {
		batman_if = list_entry( list_pos, struct batman_if, list );

		if (compare_orig(ethhdr->h_source, batman_if->net_dev->dev_addr))
			is_my_addr = 1;

		if (compare_orig(batman_packet->orig, batman_if->net_dev->dev_addr))
			is_my_orig = 1;

		if (compare_orig(batman_packet->old_orig, batman_if->net_dev->dev_addr))
			is_my_oldorig = 1;

		if (compare_orig(ethhdr->h_source, broadcastAddr))
			is_broadcast = 1;
	}

	if (batman_packet->version != COMPAT_VERSION) {

		debug_log(LOG_TYPE_ROUTING, "Drop packet: incompatible batman version (%i) \n", batman_packet->version);

	} else if (is_my_addr) {

		debug_log(LOG_TYPE_ROUTING, "Drop packet: received my own broadcast (sender: %s) \n", neigh_str);

	} else if (is_broadcast) {

		debug_log(LOG_TYPE_ROUTING, "Drop packet: ignoring all packets with broadcast source addr (sender: %s) \n", neigh_str);

	} else if (is_my_orig) {

		orig_neigh_node = get_orig_node(ethhdr->h_source);

		/* neighbour has to indicate direct link and it has to come via the corresponding interface */
		/* if received seqno equals last send seqno save new seqno for bidirectional check */
		if (has_directlink_flag && (batman_packet->seqno - if_incoming_seqno + 1 == 0)) {
			bit_mark((TYPE_OF_WORD *)&(orig_neigh_node->bcast_own[if_incoming->if_num * NUM_WORDS]), 0);
			orig_neigh_node->bcast_own_sum[if_incoming->if_num] = bit_packet_count((TYPE_OF_WORD *)&(orig_neigh_node->bcast_own[if_incoming->if_num * NUM_WORDS]));
		}

		debug_log(LOG_TYPE_ROUTING, "Drop packet: originator packet from myself (via neighbour) \n");

	} else if (has_unidirectional_flag) {

		count_real_packets(ethhdr, batman_packet, if_incoming);

		debug_log(LOG_TYPE_ROUTING, "Drop packet: originator packet with unidirectional flag \n");

	} else if (batman_packet->tq == 0) {

		count_real_packets(ethhdr, batman_packet, if_incoming);

		debug_log(LOG_TYPE_ROUTING, "Drop packet: originator packet with tq equal 0 \n");

	} else if (is_my_oldorig) {

		debug_log(LOG_TYPE_ROUTING, "Drop packet: ignoring all rebroadcast echos (sender: %s) \n", neigh_str);

	} else {

		is_duplicate = count_real_packets(ethhdr, batman_packet, if_incoming);

		orig_node = get_orig_node(batman_packet->orig);

		/* if sender is a direct neighbor the sender mac equals originator mac */
		orig_neigh_node = (is_single_hop_neigh ? orig_node : get_orig_node(ethhdr->h_source));

		/* drop packet if sender is not a direct neighbor and if we no route towards it */
		if (!is_single_hop_neigh && (orig_neigh_node->router == NULL)) {

			debug_log(LOG_TYPE_ROUTING, "Drop packet: OGM via unkown neighbor! \n");

		} else {

			is_bidirectional = isBidirectionalNeigh(orig_node, orig_neigh_node, batman_packet, if_incoming);

			/* update ranking if it is not a duplicate or has the same seqno and similar ttl as the non-duplicate */
			if (is_bidirectional && (!is_duplicate || ((orig_node->last_real_seqno == batman_packet->seqno) && (orig_node->last_ttl - 3 <= batman_packet->ttl))))
				update_orig(orig_node, ethhdr, batman_packet, if_incoming, hna_buff, hna_buff_len, is_duplicate);

			/* is single hop (direct) neighbour */
			if (is_single_hop_neigh) {

				/* mark direct link on incoming interface */
				send_forward_packet(orig_node, ethhdr, batman_packet, 0, 1, hna_buff, hna_buff_len, if_incoming);

				debug_log(LOG_TYPE_ROUTING, "Forwarding packet: rebroadcast neighbour packet with direct link flag \n");

			/* multihop originator */
			} else {

				if (is_bidirectional) {

					if (!is_duplicate) {

						send_forward_packet(orig_node, ethhdr, batman_packet, 0, 0, hna_buff, hna_buff_len, if_incoming);

						debug_log(LOG_TYPE_ROUTING, "Forwarding packet: rebroadcast originator packet \n");

					} else {

						debug_log(LOG_TYPE_ROUTING, "Drop packet: duplicate packet received\n");

					}

				} else {

					debug_log(LOG_TYPE_ROUTING, "Drop packet: not received via bidirectional link\n");

				}

			}

		}

	}
}

int packet_recv_thread(void *data)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	struct kvec iov;
	struct msghdr msg;
	struct ethhdr *ethhdr;
	struct batman_packet *batman_packet;
	unsigned char packet_buff[2000];
	unsigned int flags = MSG_DONTWAIT;	/* non-blocking */
	int result;

	iov.iov_base = packet_buff;
	iov.iov_len = sizeof(packet_buff);
	msg.msg_flags = flags;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;

	atomic_set(&data_ready_cond, 0);
	atomic_set(&exit_cond, 0);

	while ((!kthread_should_stop()) && (!atomic_read(&exit_cond))) {

		wait_event_interruptible(thread_wait, (atomic_read(&data_ready_cond) || atomic_read(&exit_cond)));

		if (kthread_should_stop() || atomic_read(&exit_cond))
			break;

		list_for_each(list_pos, &if_list) {
			batman_if = list_entry(list_pos, struct batman_if, list);

			while ((result = kernel_recvmsg(batman_if->raw_sock, &msg, &iov, 1, sizeof(packet_buff), flags)) > 0) {

				if (result < sizeof(struct ethhdr) + 1)
					continue;

				ethhdr = (struct ethhdr *)packet_buff;
				batman_packet = (struct batman_packet *)(packet_buff + sizeof(struct ethhdr));

				/* batman packet */
				switch (batman_packet->packet_type) {
				case BAT_PACKET:

					/* drop packet if it has no batman packet payload */
					if (result < sizeof(struct ethhdr) + sizeof(struct batman_packet))
						continue;

					/* network to host order for our 16bit seqno. */
					batman_packet->seqno = ntohs(batman_packet->seqno);

					spin_lock(&orig_hash_lock);
					receive_bat_packet(ethhdr, batman_packet, packet_buff + sizeof(struct ethhdr) + sizeof(struct batman_packet), result - sizeof(struct ethhdr) - sizeof(struct batman_packet), batman_if);
					spin_unlock(&orig_hash_lock);

					break;

				/* unicast packet */
				case BAT_UNICAST:

					break;

				/* batman icmp packet */
				case BAT_ICMP:

					break;

				/* broadcast packet */
				case BAT_BCAST:

					break;
				}

			}

			if ((result < 0) && (result != -EAGAIN))
				debug_log(LOG_TYPE_CRIT, "Could not receive packet from interface %s: %i\n", batman_if->net_dev->name, result);
		}

		atomic_set(&data_ready_cond, 0);

	}

	return 0;
}

void batman_data_ready(struct sock *sk, int len)
{
	void (*data_ready)(struct sock *, int) = sk->sk_user_data;

	data_ready(sk, len);

	atomic_set(&data_ready_cond, 1);
	wake_up_interruptible(&thread_wait);
}

