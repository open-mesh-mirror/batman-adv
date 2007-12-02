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



DECLARE_WAIT_QUEUE_HEAD(thread_wait);

atomic_t data_ready_cond;
atomic_t exit_cond;



void receive_bat_packet(struct ethhdr *ethhdr, struct batman_packet *batman_packet, unsigned char *hna_buff, int hna_buff_len, struct batman_if *if_incoming)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	struct orig_node *orig_neigh_node, *orig_node;
	char orig_str[ETH_STR_LEN], old_orig_str[ETH_STR_LEN], neigh_str[ETH_STR_LEN];
	char has_unidirectional_flag, has_directlink_flag;
	int is_my_addr = 0, is_my_orig = 0, is_my_oldorig = 0, is_broadcast = 0, is_bidirectional, is_single_hop_neigh, is_duplicate;

	addr_to_string(orig_str, batman_packet->orig);
	addr_to_string(old_orig_str, batman_packet->old_orig);
	addr_to_string(neigh_str, ethhdr->h_source);

	has_unidirectional_flag = (batman_packet->flags & UNIDIRECTIONAL ? 1 : 0);
	has_directlink_flag = (batman_packet->flags & DIRECTLINK ? 1 : 0);

	is_single_hop_neigh = (compare_orig(ethhdr->h_source, batman_packet->orig) ? 1 : 0);

	debug_log(LOG_TYPE_ROUTING, "Received BATMAN packet via NB: %s, IF: %s %s (from OG: %s, via old OG: %s, seqno %d, tq %d, TTL %d, V %d, UDF %d, IDF %d) \n", neigh_str, if_incoming->net_dev->name, if_incoming->addr_str, orig_str, old_orig_str, batman_packet->seqno, batman_packet->tq, batman_packet->ttl, batman_packet->version, has_unidirectional_flag, has_directlink_flag);

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
		if (has_directlink_flag && (batman_packet->seqno - if_incoming->seqno + 1 == 0)) {
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
		if (is_single_hop_neigh && (orig_neigh_node->router == NULL)) {

			debug_log(LOG_TYPE_ROUTING, "Drop packet: OGM via unkown neighbor! \n" );

		} else {

			is_bidirectional = isBidirectionalNeigh(orig_node, orig_neigh_node, batman_packet, if_incoming, is_duplicate);

			/* update ranking if it is not a duplicate or has the same seqno and similar ttl as the non-duplicate */
			if (is_bidirectional && (!is_duplicate || ((orig_node->last_real_seqno == batman_packet->seqno) && (orig_node->last_ttl - 3 <= batman_packet->ttl))))
				update_orig(orig_node, ethhdr, batman_packet, if_incoming, hna_buff, hna_buff_len, is_duplicate);

			/* is single hop (direct) neighbour */
			if (is_single_hop_neigh) {

				/* mark direct link on incoming interface */
				send_forward_packet(orig_node, ethhdr, batman_packet, 0, 1, hna_buff, hna_buff_len, if_incoming);

				debug_log(LOG_TYPE_ROUTING, "Forward packet: rebroadcast neighbour packet with direct link flag \n" );

			/* multihop originator */
			} else {

				if (is_bidirectional) {

					if (!is_duplicate) {

						send_forward_packet(orig_node, ethhdr, batman_packet, 0, 0, hna_buff, hna_buff_len, if_incoming);

						debug_log(LOG_TYPE_ROUTING, "Forward packet: rebroadcast originator packet \n" );

					} else {

						debug_log(LOG_TYPE_ROUTING, "Drop packet: duplicate packet received\n" );

					}

				} else {

					debug_log(LOG_TYPE_ROUTING, "Drop packet: not received via bidirectional link\n" );

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
	unsigned int flags = MSG_DONTWAIT | MSG_NOSIGNAL; /* no SIGPIPE & non-blocking */
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

						receive_bat_packet(ethhdr, batman_packet, packet_buff + sizeof(struct ethhdr) + sizeof(struct batman_packet), result - sizeof(struct ethhdr) - sizeof(struct batman_packet), batman_if);
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

			if (result < 0)
				debug_log(LOG_TYPE_CRIT, "batman-adv: Could not receive packet from interface %s: %i\n", batman_if->net_dev->name, result);
		}

		printk("packet_recv_thread: thread woke up\n");
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

