/*
 * Copyright (C) 2007-2009 B.A.T.M.A.N. contributors:
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
#include "aggregation.h"
#include "send.h"
#include "routing.h"



void add_bat_packet_to_list(unsigned char *packet_buff, int packet_len, struct batman_if *if_incoming, char own_packet, unsigned long send_time)
{
	/**
	 * _aggr -> pointer to the packet we want to aggregate with
	 * _pos -> pointer to the position in the queue
	 */
	struct forw_packet *forw_packet_aggr = NULL, *forw_packet_pos = NULL;
	struct hlist_node *tmp_node;
	struct batman_packet *batman_packet;
	unsigned char directlink = (((struct batman_packet *)packet_buff)->flags & DIRECTLINK ? 1 : 0);

	/* find position for the packet in the forward queue */
	spin_lock(&forw_bat_list_lock);
	hlist_for_each_entry(forw_packet_pos, tmp_node, &forw_bat_list, list) {

		/* own packets are not to be aggregated */
		if ((atomic_read(&aggregation_enabled)) && (!own_packet)) {

			/* don't save aggregation position if aggregation is disabled */
			forw_packet_aggr = forw_packet_pos;

			/**
			 * we can aggregate the current packet to this packet if:
			 * - the send time is within our MAX_AGGREGATION_MS time
			 * - the resulting packet wont be bigger than MAX_AGGREGATION_BYTES
			 */
			if ((time_before(send_time, forw_packet_pos->send_time)) &&
						(forw_packet_pos->packet_len + packet_len <= MAX_AGGREGATION_BYTES)) {

				batman_packet = (struct batman_packet *)forw_packet_pos->packet_buff;

				/**
				 * check aggregation compability
				 * -> direct link packets are broadcasted on their interface only
				 * -> aggregate packet if the current packet is a "global" packet
				 *    as well as the base packet
				 */

				/* packets without direct link flag and high TTL are flooded through the net  */
				if ((!directlink) && (!(batman_packet->flags & DIRECTLINK)) && (batman_packet->ttl != 1) &&

				/* own packets originating non-primary interfaces leave only that interface */
						((!forw_packet_pos->own) || (forw_packet_pos->if_incoming->if_num == 0)))
					break;

				batman_packet = (struct batman_packet *)packet_buff;

				/* if the incoming packet is sent via this one interface only - we still can aggregate */
				if ((directlink) && (batman_packet->ttl == 1) && (forw_packet_pos->if_incoming == if_incoming))
					break;

			}

			/* could not find packet to aggregate with */
			forw_packet_aggr = NULL;

		}

	}

	/* nothing to aggregate with - either aggregation disabled or no suitable aggregation packet found */
	if (forw_packet_aggr == NULL) {

		/* the following section can run without the lock */
		spin_unlock(&forw_bat_list_lock);

		forw_packet_aggr = kmalloc(sizeof(struct forw_packet), GFP_ATOMIC);
		forw_packet_aggr->packet_buff = kmalloc(MAX_AGGREGATION_BYTES, GFP_ATOMIC);

		INIT_HLIST_NODE(&forw_packet_aggr->list);

		forw_packet_aggr->packet_len = packet_len;
		memcpy(forw_packet_aggr->packet_buff, packet_buff, forw_packet_aggr->packet_len);

		forw_packet_aggr->own = own_packet;
		forw_packet_aggr->if_incoming = if_incoming;
		forw_packet_aggr->num_packets = 0;
		forw_packet_aggr->direct_link_flags = 0;

		forw_packet_aggr->send_time = send_time;

		batman_packet = (struct batman_packet *)packet_buff;

		/* save packet direct link flag status */
		if (batman_packet->flags & DIRECTLINK)
			forw_packet_aggr->direct_link_flags = forw_packet_aggr->direct_link_flags | (1 << forw_packet_aggr->num_packets);

		/* add new packet to packet list */
		spin_lock(&forw_bat_list_lock);
		hlist_add_head(&forw_packet_aggr->list, &forw_bat_list);
		spin_unlock(&forw_bat_list_lock);

		/* start timer for this packet */
		INIT_DELAYED_WORK(&forw_packet_aggr->delayed_work, send_outstanding_bat_packet);
		queue_delayed_work(bat_event_workqueue, &forw_packet_aggr->delayed_work, send_time - jiffies);

	} else {

		batman_packet = (struct batman_packet *)packet_buff;

		memcpy(forw_packet_aggr->packet_buff + forw_packet_aggr->packet_len, packet_buff, packet_len);
		forw_packet_aggr->packet_len += packet_len;

		forw_packet_aggr->num_packets++;

		/* save packet direct link flag status */
		if (batman_packet->flags & DIRECTLINK)
			forw_packet_aggr->direct_link_flags = forw_packet_aggr->direct_link_flags | (1 << forw_packet_aggr->num_packets);

		spin_unlock(&forw_bat_list_lock);

	}
}

void receive_aggr_bat_packet(struct ethhdr *ethhdr, unsigned char *packet_buff, int packet_len, struct batman_if *if_incoming)
{
	struct batman_packet *batman_packet;
	int16_t buff_pos = 0;

	batman_packet = (struct batman_packet *)packet_buff;

	/* unpack the aggregated packets and process them one by one */
	while (aggregated_packet(buff_pos, packet_len, batman_packet->num_hna)) {

		/* network to host order for our 16bit seqno. */
		batman_packet->seqno = ntohs(batman_packet->seqno);

		receive_bat_packet(ethhdr, batman_packet, packet_buff + buff_pos + sizeof(struct batman_packet), batman_packet->num_hna * ETH_ALEN, if_incoming);

		buff_pos += sizeof(struct batman_packet) + batman_packet->num_hna * ETH_ALEN;
		batman_packet = (struct batman_packet *)(packet_buff + buff_pos);
	}
}
