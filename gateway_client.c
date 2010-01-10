/*
 * Copyright (C) 2009 B.A.T.M.A.N. contributors:
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

#include "main.h"
#include "gateway_client.h"
#include "gateway_common.h"
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_vlan.h>

LIST_HEAD(gw_list);
DEFINE_SPINLOCK(curr_gw_lock);
DEFINE_SPINLOCK(gw_list_lock);
atomic_t gw_clnt_class;
static struct gw_node *curr_gateway;

void *gw_get_selected(void)
{
	struct gw_node *curr_gateway_tmp = NULL;

	spin_lock(&curr_gw_lock);
	curr_gateway_tmp = curr_gateway;
	spin_unlock(&curr_gw_lock);

	if (!curr_gateway_tmp)
		return NULL;

	return curr_gateway_tmp->orig_node;
}

void gw_deselect(void)
{
	spin_lock(&curr_gw_lock);
	curr_gateway = NULL;
	spin_unlock(&curr_gw_lock);
}

void gw_election(void)
{
	struct gw_node *gw_node, *curr_gw_tmp = NULL;
	uint8_t max_tq = 0;
	uint32_t max_gw_factor = 0, tmp_gw_factor = 0;
	int down, up;

	/**
	 * The batman daemon checks here if we already passed a full originator
	 * cycle in order to make sure we don't choose the first gateway we
	 * hear about. This check is based on the daemon's uptime which we
	 * don't have.
	 **/
	if (atomic_read(&gw_clnt_class) == 0)
		return;

	if (curr_gateway)
		return;

	rcu_read_lock();
	if (list_empty(&gw_list)) {
		rcu_read_unlock();

		if (curr_gateway) {
			bat_dbg(DBG_BATMAN,
				"Removing selected gateway - no gateway in range\n");
			gw_deselect();
		}

		return;
	}

	list_for_each_entry_rcu(gw_node, &gw_list, list) {
		if (!gw_node->orig_node->router)
			continue;

		if (gw_node->deleted)
			continue;

		switch (atomic_read(&gw_clnt_class)) {
		case 1: /* fast connection */
			gw_srv_class_to_kbit(gw_node->orig_node->gw_flags,
					     &down, &up);

			tmp_gw_factor = (gw_node->orig_node->router->tq_avg *
					 gw_node->orig_node->router->tq_avg *
					 down * 100 * 100) /
					 (TQ_LOCAL_WINDOW_SIZE *
					 TQ_LOCAL_WINDOW_SIZE * 64);

			if ((tmp_gw_factor > max_gw_factor) ||
			    ((tmp_gw_factor == max_gw_factor) &&
			     (gw_node->orig_node->router->tq_avg > max_tq)))
				curr_gw_tmp = gw_node;
			break;

		default: /**
			  * 2:  stable connection (use best statistic)
			  * 3:  fast-switch (use best statistic but change as
			  *     soon as a better gateway appears)
			  * XX: late-switch (use best statistic but change as
			  *     soon as a better gateway appears which has
			  *     $routing_class more tq points)
			  **/
			if (gw_node->orig_node->router->tq_avg > max_tq)
				curr_gw_tmp = gw_node;
			break;
		}

		if (gw_node->orig_node->router->tq_avg > max_tq)
			max_tq = gw_node->orig_node->router->tq_avg;

		if (tmp_gw_factor > max_gw_factor)
			max_gw_factor = tmp_gw_factor;
	}
	rcu_read_unlock();

	spin_lock(&curr_gw_lock);
	if (curr_gateway != curr_gw_tmp) {
		if ((curr_gateway) && (!curr_gw_tmp))
			bat_dbg(DBG_BATMAN,
				"Removing selected gateway - no gateway in range\n");
		else if ((!curr_gateway) && (curr_gw_tmp))
			bat_dbg(DBG_BATMAN,
				"Adding route to gateway %pM (gw_flags: %i, tq: %i)\n",
				curr_gw_tmp->orig_node->orig,
				curr_gw_tmp->orig_node->gw_flags,
				curr_gw_tmp->orig_node->router->tq_avg);
		else
			bat_dbg(DBG_BATMAN,
				"Changing route to gateway %pM (gw_flags: %i, tq: %i)\n",
				curr_gw_tmp->orig_node->orig,
				curr_gw_tmp->orig_node->gw_flags,
				curr_gw_tmp->orig_node->router->tq_avg);

		curr_gateway = curr_gw_tmp;
	}
	spin_unlock(&curr_gw_lock);
}

void gw_check_election(struct orig_node *orig_node)
{
	struct gw_node *curr_gateway_tmp;
	uint8_t gw_tq_avg, orig_tq_avg;

	spin_lock(&curr_gw_lock);
	curr_gateway_tmp = curr_gateway;
	spin_unlock(&curr_gw_lock);

	if (!curr_gateway_tmp)
		return;

	if (!curr_gateway_tmp->orig_node)
		goto deselect;

	if (!curr_gateway_tmp->orig_node->router)
		goto deselect;

	/* this node already is the gateway */
	if (curr_gateway_tmp->orig_node == orig_node)
		return;

	if (!orig_node->router)
		return;

	gw_tq_avg = curr_gateway_tmp->orig_node->router->tq_avg;
	orig_tq_avg = orig_node->router->tq_avg;

	/* the TQ value has to be better */
	if (orig_tq_avg < gw_tq_avg)
		return;

	/**
	 * if the routing class is greater than 3 the value tells us how much
	 * greater the TQ value of the new gateway must be
	 **/
	if ((atomic_read(&gw_clnt_class) > 3) &&
	    (orig_tq_avg - gw_tq_avg < atomic_read(&gw_clnt_class)))
		return;

	bat_dbg(DBG_BATMAN,
		"Restarting gateway selection: better gateway found (tq curr: %i, tq new: %i) \n",
		gw_tq_avg, orig_tq_avg);

deselect:
	gw_deselect();
}

static void gw_node_add(struct orig_node *orig_node, uint8_t new_gwflags)
{
	struct gw_node *gw_node;
	int down, up;

	gw_node = kmalloc(sizeof(struct gw_node), GFP_ATOMIC);
	if (!gw_node)
		return;

	memset(gw_node, 0, sizeof(struct gw_node));
	INIT_LIST_HEAD(&gw_node->list);
	gw_node->orig_node = orig_node;

	list_add_tail_rcu(&gw_node->list, &gw_list);

	gw_srv_class_to_kbit(new_gwflags, &down, &up);
	bat_dbg(DBG_BATMAN,
		"Found new gateway %pM -> gw_class: %i - %i%s/%i%s\n",
		orig_node->orig, new_gwflags,
		(down > 2048 ? down / 1024 : down),
		(down > 2048 ? "MBit" : "KBit"),
		(up > 2048 ? up / 1024 : up),
		(up > 2048 ? "MBit" : "KBit"));
}

void gw_node_update(struct orig_node *orig_node, uint8_t new_gwflags)
{
	struct gw_node *gw_node;

	rcu_read_lock();
	list_for_each_entry_rcu(gw_node, &gw_list, list) {
		if (gw_node->orig_node != orig_node)
			continue;

		bat_dbg(DBG_BATMAN,
			"Gateway class of originator %pM changed from %i to %i\n",
			orig_node->orig, gw_node->orig_node->gw_flags,
			new_gwflags);

		gw_node->deleted = 0;

		if (new_gwflags == 0) {
			gw_node->deleted = jiffies;
			bat_dbg(DBG_BATMAN,
				"Gateway %pM removed from gateway list\n",
				orig_node->orig);

			if (gw_node == curr_gateway)
				gw_deselect();
		}

		return;
	}
	rcu_read_unlock();

	if (new_gwflags == 0)
		return;

	gw_node_add(orig_node, new_gwflags);
}

void gw_node_delete(struct orig_node *orig_node)
{
	return gw_node_update(orig_node, 0);
}

static void gw_node_free(struct rcu_head *rcu)
{
	struct gw_node *gw_node = container_of(rcu, struct gw_node, rcu);
	kfree(gw_node);
}

void gw_node_purge_deleted(void)
{
	struct gw_node *gw_node, *gw_node_tmp;
	unsigned long timeout = (2 * PURGE_TIMEOUT * HZ) / 1000;

	spin_lock(&gw_list_lock);

	list_for_each_entry_safe(gw_node, gw_node_tmp, &gw_list, list) {
		if ((gw_node->deleted) &&
		    (time_after(jiffies, gw_node->deleted + timeout))) {

			list_del_rcu(&gw_node->list);
			call_rcu(&gw_node->rcu, gw_node_free);
		}
	}

	spin_unlock(&gw_list_lock);
}

void gw_node_list_free(void)
{
	struct gw_node *gw_node, *gw_node_tmp;

	spin_lock(&gw_list_lock);

	list_for_each_entry_safe(gw_node, gw_node_tmp, &gw_list, list) {
		list_del_rcu(&gw_node->list);
		call_rcu(&gw_node->rcu, gw_node_free);
	}

	gw_deselect();
	spin_unlock(&gw_list_lock);
}

static int _write_buffer_text(unsigned char *buff, int bytes_written,
			      struct gw_node *gw_node)
{
	int down, up;
	char gw_str[ETH_STR_LEN], router_str[ETH_STR_LEN];

	addr_to_string(gw_str, gw_node->orig_node->orig);
	addr_to_string(router_str, gw_node->orig_node->router->addr);
	gw_srv_class_to_kbit(gw_node->orig_node->gw_flags, &down, &up);

	return sprintf(buff + bytes_written,
		       "%s %-17s (%3i) %17s [%10s]: %3i - %i%s/%i%s\n",
		       (curr_gateway == gw_node ? "=>" : "  "),
		       gw_str,
		       gw_node->orig_node->router->tq_avg,
		       router_str,
		       gw_node->orig_node->router->if_incoming->dev,
		       gw_node->orig_node->gw_flags,
		       (down > 2048 ? down / 1024 : down),
		       (down > 2048 ? "MBit" : "KBit"),
		       (up > 2048 ? up / 1024 : up),
		       (up > 2048 ? "MBit" : "KBit"));
}

int gw_client_fill_buffer_text(unsigned char *buff, int buff_len)
{
	struct gw_node *gw_node;
	int bytes_written = 0, gw_count = 0;

	rcu_read_lock();
	list_for_each_entry_rcu(gw_node, &gw_list, list) {
		if (gw_node->deleted)
			continue;

		if (!gw_node->orig_node->router)
			continue;

		if (buff_len < bytes_written + (2 * ETH_STR_LEN) + 30)
			break;

		bytes_written += _write_buffer_text(buff,
						    bytes_written,
						    gw_node);
		gw_count++;
	}
	rcu_read_unlock();

	if (gw_count == 0)
		sprintf(buff, "No gateways in range ... \n");

	return bytes_written;
}

bool gw_is_target(struct sk_buff *skb)
{
	struct ethhdr *ethhdr;
	struct iphdr *iphdr;
	struct udphdr *udphdr;

	if (atomic_read(&gw_mode) != GW_MODE_CLIENT)
		return false;

	if (!curr_gateway)
		return false;

	ethhdr = (struct ethhdr *)skb->data;

	if (ntohs(ethhdr->h_proto) == ETH_P_8021Q)
		ethhdr = (struct ethhdr *)(skb->data + VLAN_HLEN);

	if (ntohs(ethhdr->h_proto) != ETH_P_IP)
		return false;

	iphdr = (struct iphdr *)(((unsigned char *)ethhdr) + ETH_HLEN);

	if (iphdr->protocol != IPPROTO_UDP)
		return false;

	udphdr = (struct udphdr *)(((unsigned char *)iphdr) +
						(iphdr->ihl * 4));

	if (ntohs(udphdr->dest) != 67)
		return false;

	return true;
}
