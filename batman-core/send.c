/*
 * Copyright (C) 2007-2008 B.A.T.M.A.N. contributors:
 * Marek Lindner, Simon Wunderlich
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




#include "main.h"
#include "send.h"
#include "log.h"
#include "routing.h"
#include "translation-table.h"
#include "hard-interface.h"
#include "types.h"
#include "vis.h"



static DECLARE_DELAYED_WORK(send_own_packet_wq, send_own_packet_work);

void start_bcast_timer(void)
{
	queue_delayed_work(bat_event_workqueue, &send_own_packet_wq, (((atomic_read(&originator_interval) - JITTER + (random32() % 2*JITTER)) * HZ) / 1000));
}

void stop_bcast_timer(void)
{
	cancel_rearming_delayed_work(&send_own_packet_wq);
}

/* sends a raw packet. */
void send_raw_packet(unsigned char *pack_buff, int pack_buff_len, uint8_t *src_addr, uint8_t *dst_addr, struct batman_if *batman_if)
{
	struct ethhdr *ethhdr;
	struct sk_buff *skb;
	int retval;
	char *data;

	if (batman_if->if_active != IF_ACTIVE)
		return;

	if (!(batman_if->net_dev->flags & IFF_UP)) {
		debug_log(LOG_TYPE_WARN, "Interface %s is not up - can't send packet via that interface !\n", batman_if->dev);
		batman_if->if_active = IF_TO_BE_DEACTIVATED;
		return;
	}

	skb = dev_alloc_skb(pack_buff_len + sizeof(struct ethhdr));
	if (!skb) 
		return;
	data = skb_put(skb, pack_buff_len + sizeof(struct ethhdr));

	memcpy(data + sizeof(struct ethhdr), pack_buff, pack_buff_len);

	ethhdr = (struct ethhdr *) data;
	memcpy(ethhdr->h_source, batman_if->net_dev->dev_addr, ETH_ALEN);
	memcpy(ethhdr->h_dest, dst_addr, ETH_ALEN);
	ethhdr->h_proto = htons(ETH_P_BATMAN);

	skb_reset_mac_header(skb);
	skb->network_header = skb->mac_header + ETH_HLEN;
	skb->priority = TC_PRIO_CONTROL;
	skb->protocol = __constant_htons(ETH_P_BATMAN);
	skb->dev = batman_if->net_dev;

	/* dev_queue_xmit() returns a negative result on error.
	 * However on congestion and traffic shaping, it drops and returns 
	 * NET_XMIT_DROP (which is > 0). This will not be treated as an error. */
	retval = dev_queue_xmit(skb);
	if (retval < 0) {
		debug_log(LOG_TYPE_CRIT, "Can't write to raw socket: %i\n", retval);
		batman_if->if_active = IF_TO_BE_DEACTIVATED;
	}       
}

/* send a batman packet. */
static void send_packet(unsigned char *pack_buff, int pack_buff_len, struct batman_if *if_outgoing, char own_packet)
{
	struct batman_if *batman_if;
	char orig_str[ETH_STR_LEN];
	char directlink = (((struct batman_packet *)pack_buff)->flags & DIRECTLINK ? 1 : 0);

	/* according to calltree, outgoing iface should always be specified. */
	if (if_outgoing == NULL) {
		debug_log(LOG_TYPE_CRIT, "Error - can't forward packet: outgoing iface not specified\n");
		return;
	}
	if (if_outgoing->if_active != IF_ACTIVE)
		return;

	addr_to_string(orig_str, ((struct batman_packet *)pack_buff)->orig);

	/* multihomed peer assumed */
	if (directlink && (((struct batman_packet *)pack_buff)->ttl == 1)) {
		send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);
	} else {

		/* non-primary OGMs are only broadcasted on their interface */
		if (own_packet && (if_outgoing->if_num > 0)) {
			debug_log(LOG_TYPE_BATMAN, "Sending own packet (originator %s, seqno %d, TTL %d) on interface %s [%s]\n", orig_str, ntohs(((struct batman_packet *)pack_buff)->seqno), ((struct batman_packet *)pack_buff)->ttl, if_outgoing->dev, if_outgoing->addr_str);
			send_raw_packet(pack_buff, pack_buff_len, if_outgoing->net_dev->dev_addr, broadcastAddr, if_outgoing);
		} else {
			rcu_read_lock();
			list_for_each_entry_rcu(batman_if, &if_list, list) {
				if (batman_if->if_active != IF_ACTIVE)
					continue;

				if (directlink && (if_outgoing == batman_if))
					((struct batman_packet *)pack_buff)->flags |= DIRECTLINK;
				else
					((struct batman_packet *)pack_buff)->flags &= ~DIRECTLINK;

				debug_log(LOG_TYPE_BATMAN, "%s packet (originator %s, seqno %d, TTL %d) on interface %s [%s]\n", (own_packet ? "Sending own" : "Forwarding"), orig_str, ntohs(((struct batman_packet *)pack_buff)->seqno), ((struct batman_packet *)pack_buff)->ttl, batman_if->dev, batman_if->addr_str);

				send_raw_packet(pack_buff, pack_buff_len, batman_if->net_dev->dev_addr, broadcastAddr, batman_if);
			}
			rcu_read_unlock();

		}
	}
}
void send_own_packet_work(struct work_struct *work)
{
	struct batman_if *batman_if;

	rcu_read_lock();
	list_for_each_entry_rcu(batman_if, &if_list, list) 
		send_own_packet(batman_if);
	rcu_read_unlock();

	start_bcast_timer();
}

void send_own_packet(struct batman_if *batman_if)
{
	unsigned char *buff_ptr;

	/* if local hna has changed and interface is a primary interface */
	if ((hna_local_changed) && (batman_if->if_num == 0)) {

		buff_ptr = batman_if->pack_buff;

		batman_if->pack_buff_len = sizeof(struct batman_packet) + (num_hna * ETH_ALEN);
		batman_if->pack_buff = kmalloc(batman_if->pack_buff_len, GFP_KERNEL);
		memcpy(batman_if->pack_buff, buff_ptr, sizeof(struct batman_packet));

		((struct batman_packet *)(batman_if->pack_buff))->num_hna = hna_local_fill_buffer(batman_if->pack_buff + sizeof(struct batman_packet), batman_if->pack_buff_len - sizeof(struct batman_packet));

		kfree(buff_ptr);
	}

	/* change sequence number to network order */
	((struct batman_packet *)batman_if->pack_buff)->seqno = htons(batman_if->seqno);
	if (is_vis_server())
		((struct batman_packet *)batman_if->pack_buff)->flags = VIS_SERVER;		
	else
		((struct batman_packet *)batman_if->pack_buff)->flags = 0;		

	/* could be read by receive_bat_packet() */
	spin_lock(&batman_if->seqno_lock);
	batman_if->seqno++;
	spin_unlock(&batman_if->seqno_lock);

	slide_own_bcast_window(batman_if);
	send_packet(batman_if->pack_buff, batman_if->pack_buff_len, batman_if, 1);

}

void send_forward_packet(struct orig_node *orig_node, struct ethhdr *ethhdr, struct batman_packet *batman_packet, uint8_t idf, unsigned char *hna_buff, int hna_buff_len, struct batman_if *if_outgoing)
{
	unsigned char in_tq, in_ttl, tq_avg = 0;

	if (batman_packet->ttl <= 1) {
		debug_log(LOG_TYPE_BATMAN, "ttl exceeded \n");
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

	}

	/* apply hop penalty */
	batman_packet->tq = (batman_packet->tq * (TQ_MAX_VALUE - TQ_HOP_PENALTY)) / (TQ_MAX_VALUE);

	debug_log(LOG_TYPE_BATMAN, "Forwarding packet: tq_orig: %i, tq_avg: %i, tq_forw: %i, ttl_orig: %i, ttl_forw: %i \n", in_tq, tq_avg, batman_packet->tq, in_ttl - 1, batman_packet->ttl);

	batman_packet->seqno = htons(batman_packet->seqno);

	if (idf)
		batman_packet->flags |= DIRECTLINK;
	else
		batman_packet->flags &= ~DIRECTLINK;

	send_packet((unsigned char *)batman_packet, sizeof(struct batman_packet) + hna_buff_len, if_outgoing, 0);
}
