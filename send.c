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

#include "compat.h"

static void send_outstanding_packets(struct work_struct *work);

static DECLARE_DELAYED_WORK(send_outstanding_packets_wq, send_outstanding_packets);
spinlock_t packets_timer_lock = SPIN_LOCK_UNLOCKED;
static unsigned long send_time_next = 0;


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
	ethhdr->h_proto = __constant_htons(ETH_P_BATMAN);

	skb_reset_mac_header(skb);
	skb_set_network_header(skb, ETH_HLEN);
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

/* send a batman packet */
static void send_packet(struct forw_packet *forw_packet)
{
	struct batman_if *batman_if;
	struct batman_packet *batman_packet = (struct batman_packet *)(forw_packet->packet_buff);
	char orig_str[ETH_STR_LEN];
	unsigned char directlink = (batman_packet->flags & DIRECTLINK ? 1 : 0);
	uint8_t packet_num;
	int16_t buff_pos;

	/* according to calltree the outgoing iface should always be specified. */
	if (forw_packet->if_outgoing == NULL) {
		debug_log(LOG_TYPE_CRIT, "Error - can't forward packet: outgoing iface not specified\n");
		return;
	}

	if (forw_packet->if_outgoing->if_active != IF_ACTIVE)
		return;

	addr_to_string(orig_str, batman_packet->orig);

	/* multihomed peer assumed */
	/* non-primary OGMs are only broadcasted on their interface */
	if ((directlink && (batman_packet->ttl == 1)) ||
		    (forw_packet->own && (forw_packet->if_outgoing->if_num > 0))) {

		/* FIXME: what about aggregated packets ? */
		debug_log(LOG_TYPE_BATMAN, "%s packet (originator %s, seqno %d, TTL %d) on interface %s [%s]\n", (forw_packet->own ? "Sending own" : "Forwarding"), orig_str, ntohs(batman_packet->seqno), batman_packet->ttl, forw_packet->if_outgoing->dev, forw_packet->if_outgoing->addr_str);

		send_raw_packet(forw_packet->packet_buff, forw_packet->packet_len, forw_packet->if_outgoing->net_dev->dev_addr, broadcastAddr, forw_packet->if_outgoing);
		return;
	}

	/* broadcast on every interface */
	rcu_read_lock();
	list_for_each_entry_rcu(batman_if, &if_list, list) {
		if (batman_if->if_active != IF_ACTIVE)
			continue;

		packet_num = buff_pos = 0;
		batman_packet = (struct batman_packet *)(forw_packet->packet_buff);

		/* adjust all flags and log packets */
		while (aggregated_packet(buff_pos, forw_packet->packet_len, batman_packet->num_hna)) {

			/* we might have aggregated direct link packets with an ordinary base packet */
			if ((forw_packet->direct_link_flags & (1 << packet_num)) && (forw_packet->if_outgoing == batman_if))
				batman_packet->flags |= DIRECTLINK;
			else
				batman_packet->flags &= ~DIRECTLINK;

			/* for later logging */
			if (packet_num > 0)
				addr_to_string(orig_str, batman_packet->orig);

			debug_log(LOG_TYPE_BATMAN, "%s %spacket (originator %s, seqno %d, TTL %d, IDF %s) on interface %s [%s]\n", (packet_num > 0 ? "Forwarding" : (forw_packet->own ? "Sending own" : "Forwarding")), (packet_num > 0 ? "aggregated " : ""), orig_str, ntohs(batman_packet->seqno), batman_packet->ttl, (batman_packet->flags & DIRECTLINK ? "on" : "off"), batman_if->dev, batman_if->addr_str);

			buff_pos += sizeof(struct batman_packet) + batman_packet->num_hna * ETH_ALEN;
			packet_num++;
			batman_packet = (struct batman_packet *)(forw_packet->packet_buff + buff_pos);
		}

		send_raw_packet(forw_packet->packet_buff, forw_packet->packet_len, batman_if->net_dev->dev_addr, broadcastAddr, batman_if);
	}
	rcu_read_unlock();
}

void set_outstanding_packets_timer(unsigned long send_time)
{
	/**
	 * on scheduler start or after a complete run through send_outstanding_packets()
	 * or if first packet was just sent or if the new packet
	 * needs to be sent earlier than the previously scheduled packet
	 */
	if ((send_time_next == 0) || (time_after_eq(jiffies, send_time_next)) || (time_before(send_time, send_time_next))) {

		/* FIXME: do we need to make send_time_next thread safe ?? */
		send_time_next = send_time;

		/**
		 * if we are being called by send_outstanding_packets() we can't acquire the lock
		 * because we should not kill the running function that sends the packets
		 */
		if (spin_trylock(&packets_timer_lock)) {
			cancel_delayed_work_sync(&send_outstanding_packets_wq);

			queue_delayed_work(bat_event_workqueue, &send_outstanding_packets_wq, send_time_next - jiffies);
			spin_unlock(&packets_timer_lock);
		}
	}
}

static void add_packet_to_list(unsigned char *packet_buff, int packet_len, struct batman_if *if_outgoing, char own_packet, unsigned long send_time)
{
	/**
	 * _new -> new forward packet which might be created
	 * _aggr -> pointer to the packet we want to aggregate with
	 * _pos -> pointer to the position in the queue
	 */
	struct forw_packet *forw_packet_new, *forw_packet_aggr = NULL, *forw_packet_pos = NULL;
	struct hlist_node *tmp_node;
	struct batman_packet *batman_packet;

	/* find position for the packet in the forward queue */
	rcu_read_lock();
	hlist_for_each_entry_rcu(forw_packet_pos, tmp_node, &forw_list, list) {

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
				 */

				/* packets without direct link flag and high TTL are flooded through the net  */
				if (((!(batman_packet->flags & DIRECTLINK)) && (batman_packet->ttl != 1)) &&

				/* own packets originating non-primary interfaces leave only that interface */
						((!forw_packet_pos->own) || (forw_packet_pos->if_outgoing->if_num == 0)))
					break;

				/**
				 * FIXME: if we can aggregate this packet with an ordinary packet we flood it over
				 *        all interfaces  - if its a direct link base packet only via one interface
				 *        whats correct ?
				 */
				batman_packet = (struct batman_packet *)packet_buff;

				/* if the incoming packet is sent via this one interface only - we still can aggregate */
				if ((batman_packet->flags & DIRECTLINK) && (batman_packet->ttl == 1) && (forw_packet_pos->if_outgoing == if_outgoing))
					break;

			}

			/* could not find packet to aggregate with */
			forw_packet_aggr = NULL;

		}

		if (time_after(send_time, forw_packet_pos->send_time))
			break;

	}
	rcu_read_unlock();

	/* nothing to aggregate with - either aggregation disabled or no suitable aggregation packet found */
	if (forw_packet_aggr == NULL) {

		forw_packet_new = kmalloc(sizeof(struct forw_packet), GFP_ATOMIC);
		forw_packet_new->packet_buff = kmalloc(MAX_AGGREGATION_BYTES, GFP_ATOMIC);

		INIT_HLIST_NODE(&forw_packet_new->list);
		INIT_RCU_HEAD(&forw_packet_new->rcu);

		forw_packet_new->packet_len = packet_len;
		memcpy(forw_packet_new->packet_buff, packet_buff, forw_packet_new->packet_len);

		forw_packet_new->own = own_packet;
		forw_packet_new->if_outgoing = if_outgoing;
		forw_packet_new->num_packets = 0;
		forw_packet_new->direct_link_flags = 0;

		forw_packet_new->send_time = send_time;

	} else {

		memcpy(forw_packet_aggr->packet_buff + forw_packet_aggr->packet_len, packet_buff, packet_len);
		forw_packet_aggr->packet_len += packet_len;

		forw_packet_aggr->num_packets++;

		forw_packet_new = forw_packet_aggr;

	}

	batman_packet = (struct batman_packet *)packet_buff;

	/* save packet direct link flag status */
	if (batman_packet->flags & DIRECTLINK)
		forw_packet_new->direct_link_flags = forw_packet_new->direct_link_flags | (1 << forw_packet_new->num_packets);

	/* if the packet was not aggregated */
	if (forw_packet_aggr == NULL) {
		spin_lock(&forw_list_lock);

		/* no packet in the queue */
		if (forw_packet_pos == NULL)
			hlist_add_head_rcu(&forw_packet_new->list, &forw_list);

		/* add new packet before the found item */
		else if (time_before(forw_packet_new->send_time, forw_packet_pos->send_time))
			hlist_add_before_rcu(&forw_packet_new->list, &forw_packet_pos->list);

		/* add new packet after the found item */
		else
			hlist_add_after_rcu(&forw_packet_pos->list, &forw_packet_new->list);

		spin_unlock(&forw_list_lock);

		set_outstanding_packets_timer(forw_packet_new->send_time);
	}
}

void schedule_own_packet(struct batman_if *batman_if)
{
	unsigned char *new_buff;
	unsigned long send_time;
	struct batman_packet *batman_packet;
	int new_len;

	batman_packet = (struct batman_packet *)batman_if->packet_buff;

	/* if local hna has changed and interface is a primary interface */
	if ((hna_local_changed) && (batman_if->if_num == 0)) {

		new_len = sizeof(struct batman_packet) + (num_hna * ETH_ALEN);
		new_buff = kmalloc(new_len, GFP_ATOMIC);

		/* keep old buffer if kmalloc should fail */
		if (new_buff) {
			memcpy(new_buff, batman_if->packet_buff, sizeof(struct batman_packet));
			batman_packet = (struct batman_packet *)new_buff;

			batman_packet->num_hna = hna_local_fill_buffer(
						new_buff + sizeof(struct batman_packet),
						new_len - sizeof(struct batman_packet));

			kfree(batman_if->packet_buff);
			batman_if->packet_buff = new_buff;
			batman_if->packet_len = new_len;
		}

	}

	/* change sequence number to network order */
	batman_packet->seqno = htons((uint16_t)atomic_read(&batman_if->seqno));

	if (is_vis_server())
		batman_packet->flags = VIS_SERVER;
	else
		batman_packet->flags = 0;

	/* could be read by receive_bat_packet() */
	atomic_inc(&batman_if->seqno);

	slide_own_bcast_window(batman_if);
	send_time = jiffies + (((atomic_read(&originator_interval) - JITTER + (random32() % 2*JITTER)) * HZ) / 1000);
	add_packet_to_list(batman_if->packet_buff, batman_if->packet_len, batman_if, 1, send_time);
}

void schedule_forward_packet(struct orig_node *orig_node, struct ethhdr *ethhdr, struct batman_packet *batman_packet, uint8_t directlink, unsigned char *hna_buff, int hna_buff_len, struct batman_if *if_outgoing)
{
	unsigned char in_tq, in_ttl, tq_avg = 0;
	unsigned long send_time;

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

	if (directlink)
		batman_packet->flags |= DIRECTLINK;
	else
		batman_packet->flags &= ~DIRECTLINK;

	if (atomic_read(&aggregation_enabled))
		send_time = jiffies + (((MAX_AGGREGATION_MS - (JITTER/2) + (random32() % JITTER)) * HZ) / 1000);
	else
		send_time = jiffies + (((random32() % (JITTER/2)) * HZ) / 1000);

	add_packet_to_list((unsigned char *)batman_packet, sizeof(struct batman_packet) + hna_buff_len, if_outgoing, 0, send_time);
}

static void forw_packet_free(struct rcu_head *rcu)
{
	struct forw_packet *forw_packet = container_of(rcu, struct forw_packet, rcu);

	kfree(forw_packet->packet_buff);
	kfree(forw_packet);
}

static void send_outstanding_packets(struct work_struct *work)
{
	struct forw_packet *forw_packet;
	struct hlist_node *tmp_node;

	spin_lock(&packets_timer_lock);

	hlist_for_each_entry_rcu(forw_packet, tmp_node, &forw_list, list) {

		/* FIXME: is that safe ? */
		if (time_after(forw_packet->send_time, jiffies)) {
			set_outstanding_packets_timer(forw_packet->send_time);
			break;
		}

		send_packet(forw_packet);

		spin_lock(&forw_list_lock);
		hlist_del_rcu(&forw_packet->list);
		spin_unlock(&forw_list_lock);

		/**
		 * we have to have at least one packet in the queue
		 * to determine the queues wake up time
		 */
		if (forw_packet->own)
			schedule_own_packet(forw_packet->if_outgoing);

		call_rcu(&forw_packet->rcu, forw_packet_free);
	}

	/* send_time_next was set while running through the loop above */
	queue_delayed_work(bat_event_workqueue, &send_outstanding_packets_wq, send_time_next - jiffies);

	spin_unlock(&packets_timer_lock);
}

void purge_outstanding_packets(void)
{
	struct forw_packet *forw_packet;
	struct hlist_node *tmp_node;

	debug_log(LOG_TYPE_BATMAN, "purge_outstanding_packets()\n");

	cancel_delayed_work_sync(&send_outstanding_packets_wq);

	hlist_for_each_entry_rcu(forw_packet, tmp_node, &forw_list, list) {

		spin_lock(&forw_list_lock);
		hlist_del_rcu(&forw_packet->list);
		spin_unlock(&forw_list_lock);

		call_rcu(&forw_packet->rcu, forw_packet_free);
	}

	/* reset timer */
	send_time_next = 0;
}
