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
#include "batman-adv-send.h"
#include "batman-adv-log.h"
#include "types.h"



void send_raw_packet(unsigned char *pack_buff, int pack_buff_len, uint8_t *send_addr, uint8_t *recv_addr, struct batman_if *batman_if)
{
	struct msghdr msg;
	struct iovec vector[2];
	struct ethhdr ethhdr;
	int retval;

	memcpy(ethhdr.h_dest, recv_addr, ETH_ALEN);
	memcpy(ethhdr.h_source, send_addr, ETH_ALEN);
	ethhdr.h_proto = htons(ETH_P_BATMAN);

	vector[0].iov_base = &ethhdr;
	vector[0].iov_len  = sizeof(struct ethhdr);
	vector[1].iov_base = pack_buff;
	vector[1].iov_len  = pack_buff_len;

	memcpy(ethhdr.h_dest, recv_addr, ETH_ALEN);
	memcpy(ethhdr.h_source, send_addr, ETH_ALEN);
	ethhdr.h_proto = htons(ETH_P_BATMAN);

	msg.msg_flags = MSG_NOSIGNAL | MSG_DONTWAIT; /* no SIGPIPE & non-blocking */
	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	/* minimum packet size is 46 bytes for ethernet */
	if ((retval = kernel_sendmsg(batman_if->raw_sock, &msg, (struct kvec *)vector, 2, pack_buff_len + sizeof(struct ethhdr))) < 0)
		debug_log(LOG_TYPE_CRIT, "Can't write to raw socket: %i\n", retval);
}

void send_packet(unsigned char *pack_buff, int pack_buff_len, struct batman_if *if_outgoing, char own_packet)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	char directlink = (((struct batman_packet *)pack_buff)->flags & DIRECTLINK ? 1 : 0);

	if (((struct batman_packet *)pack_buff)->flags & UNIDIRECTIONAL) {

		if (if_outgoing != NULL) {
			debug_log(LOG_TYPE_ROUTING, "Forwarding packet (originator %s, seqno %d, TTL %d) on interface %s\n", if_outgoing->addr_str, ntohs(((struct batman_packet *)pack_buff)->seqno), ((struct batman_packet *)pack_buff)->ttl, if_outgoing->net_dev->name);

			send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);
		} else {

			debug_log(LOG_TYPE_CRIT, "Error - can't forward packet with UDF: outgoing iface not specified \n");

		}

		/* multihomed peer assumed */
	} else if (directlink && (((struct batman_packet *)pack_buff)->ttl == 1)) {

		if (if_outgoing != NULL) {

			send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);

		} else {

			debug_log(LOG_TYPE_CRIT, "Error - can't forward packet with IDF: outgoing iface not specified (multihomed) \n");

		}

	} else {

		if (directlink && (if_outgoing == NULL)) {

			debug_log(LOG_TYPE_CRIT, "Error - can't forward packet with IDF: outgoing iface not specified \n");

		} else {

			/* non-primary interfaces are only broadcasted on their interface */
			if (own_packet && (if_outgoing->if_num > 0)) {
				debug_log(LOG_TYPE_ROUTING, "Sending own packet (originator %s, seqno %d, TTL %d) on interface %s\n", if_outgoing->addr_str, ntohs(((struct batman_packet *)pack_buff)->seqno), ((struct batman_packet *)pack_buff)->ttl, if_outgoing->net_dev->name);

				send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);
			} else {

				list_for_each(list_pos, &if_list) {
					batman_if = list_entry(list_pos, struct batman_if, list);

					if (directlink && (if_outgoing == batman_if))
						((struct batman_packet *)pack_buff)->flags = DIRECTLINK;
					else
						((struct batman_packet *)pack_buff)->flags = 0x00;

					debug_log(LOG_TYPE_ROUTING, "%s packet (originator %s, seqno %d, TTL %d) on interface %s\n", (own_packet ? "Sending own" : "Forwarding"), batman_if->addr_str, ntohs(((struct batman_packet *)pack_buff)->seqno), ((struct batman_packet *)pack_buff)->ttl, batman_if->net_dev->name);

					send_raw_packet(pack_buff, pack_buff_len, batman_if->net_dev->dev_addr, broadcastAddr, batman_if);
				}

			}

		}

	}

}

void send_own_packet(unsigned long data)
{
	struct batman_if *batman_if = (struct batman_if *)data;

	/* change sequence number to network order */
	((struct batman_packet *)batman_if->pack_buff)->seqno = htons(batman_if->seqno);

	/* could be read by receive_bat_packet() */
	spin_lock(&batman_if->seqno_lock);
	batman_if->seqno++;
	spin_unlock(&batman_if->seqno_lock);

	send_packet(batman_if->pack_buff, batman_if->pack_buff_len, batman_if, 1);

	init_timer(&batman_if->bcast_timer);

	batman_if->bcast_timer.expires = jiffies + (((atomic_read(&originator_interval) - JITTER + (random32() % 2*JITTER)) * HZ) / 1000);
	batman_if->bcast_timer.data = (unsigned long)batman_if;
	batman_if->bcast_timer.function = send_own_packet;

	add_timer(&batman_if->bcast_timer);
}

void send_forward_packet(struct orig_node *orig_node, struct ethhdr *ethhdr, struct batman_packet *batman_packet, uint8_t udf, uint8_t idf, unsigned char *hna_buff, int hna_buff_len, struct batman_if *if_outgoing)
{
	char in_tq, in_ttl, tq_avg = 0;

	if (batman_packet->ttl <= 1) {
		debug_log(LOG_TYPE_ROUTING, "ttl exceeded \n");
		return;
	}

	in_tq = batman_packet->tq;
	in_ttl = batman_packet->ttl;

	batman_packet->ttl--;
	memcpy(batman_packet->old_orig, ethhdr->h_source, ETH_ALEN);

	/* rebroadcast tq of our best ranking neighbor to ensure the rebroadcast of our best tq value */
	if ((orig_node->router != NULL) && (orig_node->router->tq_avg != 0)) {

		/* rebroadcast ogm of best ranking neighbor as is */
		if (!compare_orig(orig_node->router->addr, ethhdr->h_source)) {

			batman_packet->tq = orig_node->router->tq_avg;
			batman_packet->ttl = orig_node->router->last_ttl - 1;

		}

		tq_avg = orig_node->router->tq_avg;

		if ((orig_node->router->orig_node->tq_own > TQ_MAX_VALUE - PERFECT_TQ_PENALTY) && (orig_node->router->orig_node->tq_asym_penality > TQ_MAX_VALUE - PERFECT_TQ_PENALTY))
			batman_packet->tq -= PERFECT_TQ_PENALTY;

	}

	debug_log(LOG_TYPE_ROUTING, "Forwarding packet: tq_orig: %i, tq_avg: %i, tq_forw: %i, ttl_orig: %i, ttl_forw: %i \n", in_tq, tq_avg, batman_packet->tq, in_ttl - 1, batman_packet->ttl);

	batman_packet->seqno = htons(batman_packet->seqno);

	if (udf)
		batman_packet->flags = (UNIDIRECTIONAL | DIRECTLINK);
	else if (idf)
		batman_packet->flags = DIRECTLINK;
	else
		batman_packet->flags = 0x00;

	send_packet((unsigned char *)batman_packet, sizeof(struct batman_packet) + hna_buff_len, if_outgoing, 0);
}
