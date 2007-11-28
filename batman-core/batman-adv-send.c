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

	if ((retval = kernel_sendmsg(batman_if->raw_sock, &msg, (struct kvec *)vector, 2, pack_buff_len + sizeof(struct ethhdr))) < 0)
		printk("batman-adv: Can't write to raw socket: %i\n", retval);
}

void send_packet(unsigned char *pack_buff, int pack_buff_len, struct batman_if *if_outgoing, char own_packet)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	char directlink = (((struct batman_packet *)pack_buff)->flags & DIRECTLINK ? 1 : 0);


	/* change sequence number to network order */
	((struct batman_packet *)pack_buff)->seqno = htons(((struct batman_packet *)pack_buff)->seqno);

	if (((struct batman_packet *)pack_buff)->flags & UNIDIRECTIONAL) {

		if (if_outgoing != NULL) {

// 			debug_output( 4, "Forwarding packet (originator %s, seqno %d, TTL %d) on interface %s\n", orig_str, ntohs( ((struct bat_packet *)forw_node->pack_buff)->seqno ), ((struct bat_packet *)forw_node->pack_buff)->ttl, forw_node->if_outgoing->dev );

			send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);

		} else {

// 			debug_output( 0, "Error - can't forward packet with UDF: outgoing iface not specified \n" );

		}

		/* multihomed peer assumed */
	} else if ((directlink) && (((struct batman_packet *)pack_buff)->ttl == 1)) {

		if (if_outgoing != NULL) {

			send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);

		} else {

// 			debug_output( 0, "Error - can't forward packet with IDF: outgoing iface not specified (multihomed) \n" );

		}

	} else {

		if ((directlink) && (if_outgoing == NULL)) {

// 			debug_output( 0, "Error - can't forward packet with IDF: outgoing iface not specified \n" );

		} else {

			/* non-primary interfaces are only broadcasted on their interface */
			if ((own_packet) && (if_outgoing->if_num > 0)) {

// 				debug_output( 4, "Forwarding packet (originator %s, seqno %d, TTL %d) on interface %s\n", orig_str, ntohs( ((struct bat_packet *)forw_node->pack_buff)->seqno ), ((struct bat_packet *)forw_node->pack_buff)->ttl, forw_node->if_outgoing->dev );

				send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);

			} else {

				list_for_each(list_pos, &if_list) {

					batman_if = list_entry(list_pos, struct batman_if, list);

					if ((directlink) && (if_outgoing == batman_if))
						((struct batman_packet *)pack_buff)->flags = DIRECTLINK;
					else
						((struct batman_packet *)pack_buff)->flags = 0x00;

// 					debug_output( 4, "Forwarding packet (originator %s, seqno %d, TTL %d) on interface %s\n", orig_str, ntohs( ((struct bat_packet *)forw_node->pack_buff)->seqno ), ((struct bat_packet *)forw_node->pack_buff)->ttl, batman_if->dev );

					send_raw_packet(pack_buff, pack_buff_len, batman_if->net_dev->dev_addr, broadcastAddr, batman_if);

				}

			}

		}

	}

}

void send_own_packet(unsigned long data)
{
	struct batman_if *batman_if = (struct batman_if *)data;

	send_packet((unsigned char *)&batman_if->out, sizeof(struct batman_packet), batman_if, 1);

	batman_if->out.seqno++;

	init_timer(&batman_if->bcast_timer);

	batman_if->bcast_timer.expires = jiffies + (((originator_interval - JITTER + (get_random_int() % 2*JITTER)) * HZ) / 1000);
	batman_if->bcast_timer.data = (unsigned long)batman_if;
	batman_if->bcast_timer.function = send_own_packet;

	add_timer(&batman_if->bcast_timer);
}
