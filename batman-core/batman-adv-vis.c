/*
 * Copyright (C) 2008 B.A.T.M.A.N. contributors:
 * Simon Wunderlich
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
#include "batman-adv-vis.h"
#include "batman-adv-log.h"
#include "hash.h"

struct hashtable_t *vis_hash;
DEFINE_SPINLOCK(vis_hash_lock);
static DEFINE_SPINLOCK(vis_own_packet_lock);
static struct vis_info *my_vis_info;

void free_info(void *data);
void send_vis_packet(unsigned long data);
void start_vis_timer(void);


int vis_info_cmp(void *data1, void *data2) {
	struct vis_info *d1, *d2;
	d1 = data1;
	d2 = data2;
	return(memcmp(d1->packet.vis_orig, d2->packet.vis_orig, ETH_ALEN));
}

/* hashfunction to choose an entry in a hash table of given size */
/* hash algorithm from http://en.wikipedia.org/wiki/Hash_table */
int vis_info_choose(void *data, int size) {
	struct vis_info *vis_info = data;
	unsigned char *key;
	uint32_t hash = 0;
	size_t i;

	key = vis_info->packet.vis_orig;
	for (i = 0; i < ETH_ALEN; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash%size);
}


/* handle the server sync packet, add into the hash if new. */
void receive_server_sync_packet(struct vis_packet *vis_packet, int vis_info_len) 
{
	struct vis_info *info, *old_info;
	/* ignore own packets */
	if (is_my_mac(vis_packet->vis_orig))
		return;
	/* sanity check */
	if (vis_hash == NULL)
		return;
	info = kmalloc(sizeof(struct vis_info) + vis_info_len,GFP_KERNEL);
	if (info == NULL) 
		return;

	init_timer(&info->vis_timer);
	info->last_seen = jiffies;
	memcpy(&info->packet, vis_packet, sizeof(struct vis_packet) + vis_info_len);

	/* see if the packet is already in vis_hash */
	spin_lock(&vis_hash_lock);
	old_info = hash_find(vis_hash, info);

	if (old_info != NULL) {
		if (info->packet.seqno - old_info->packet.seqno <= 0) {
			if (info->packet.seqno == info->packet.seqno) {
				/* LATER: add to receive_from_list */
			}
			/* same or newer packet is already in hash. */
			free_info(info);
			goto end;
		} 
		/* remove old entry */
		hash_remove(vis_hash, old_info);
		free_info(old_info);
	}

	/* repair if entries is longer than packet. */
	if (info->packet.entries * sizeof(struct vis_info) > vis_info_len) 
		info->packet.entries = vis_info_len / sizeof(struct vis_info);

	/* LATER: fill the list with vis_orig. */
	info->receive_from_list = NULL;

	/* try to add it */
	if (hash_add(vis_hash, info)< 0) {
		/* did not work (for some reason) */
		free_info(info);
		goto end;
	}	
	/* activate timer */
	info->vis_timer.expires = jiffies + ((random32()%100) * HZ)/1000;
	info->vis_timer.data = (unsigned long)info;
	info->vis_timer.function = send_vis_packet;
	add_timer(&info->vis_timer);

end:
	spin_unlock(&vis_hash_lock);

}

/* send own vis data */
static void generate_vis_packet(void) 
{
	struct hash_it_t *hashit = NULL;
	struct orig_node *orig_node;
	struct vis_info *info = (struct vis_info *)my_vis_info;
	struct vis_info_entry *entry;

	info->last_seen = jiffies;

	spin_lock(&orig_hash_lock);
	info->packet.seqno++;
	info->packet.entries = 0;

	while (NULL != (hashit = hash_iterate(orig_hash, hashit))) {
		orig_node = hashit->bucket->data;
		if (orig_node->router != NULL
			&& memcmp(orig_node->router->addr, orig_node->orig, ETH_ALEN) == 0
			&& orig_node->router->tq_avg > 0) {

			/* fill one entry into buffer. */
			entry = (struct vis_info_entry *)((char *)info + sizeof(struct vis_info_entry) * info->packet.entries);
			memcpy(entry->dest, orig_node->orig, ETH_ALEN);
			entry->quality = orig_node->router->tq_avg;
			info->packet.entries++;
			/* don't fill more than 1000 bytes */
			if (info->packet.entries + 1 > (1000 - sizeof(struct vis_info)) / sizeof(struct vis_info_entry)) 
				break;
		}
	}

	spin_unlock(&orig_hash_lock);
}

/* called from timer; send (and maybe generate) vis packet. */
void send_own_vis_packet(unsigned long arg) 
{

	spin_lock(&vis_own_packet_lock);

	generate_vis_packet();
	send_vis_packet((unsigned long) my_vis_info);
	start_vis_timer();

	spin_lock(&vis_own_packet_lock);
}

/* only send vis packet. called from timers and send_own_vis_packet() */
void send_vis_packet(unsigned long arg) {
	struct batman_if *batman_if;
	struct vis_info *info = (struct vis_info *)arg;
	struct list_head *list_pos;

	/* LATER: change this to send only to vis "neighbours" */
	spin_lock(&if_list_lock);
	memcpy(info->packet.target_orig, broadcastAddr, ETH_ALEN);
	memcpy(info->packet.sender_orig, ((struct batman_if *)if_list.next)->net_dev->dev_addr, ETH_ALEN);
	/* broadcast packet */
	list_for_each(list_pos, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);
		send_raw_packet((unsigned char *)&info->packet, 
				sizeof(struct vis_packet) + info->packet.entries * sizeof(struct vis_info_entry), 
				batman_if->net_dev->dev_addr, broadcastAddr, batman_if);
	}
	spin_unlock(&if_list_lock);
}

/* receive and handle a vis packet */
void receive_vis_packet(struct ethhdr *ethhdr, struct vis_packet *vis_packet, int vis_info_len)
{
	switch (vis_packet->vis_type) {
	case VIS_TYPE_SERVER_SYNC:
		receive_server_sync_packet(vis_packet, vis_info_len);
		break;
	case VIS_TYPE_CLIENT_UPDATE:
		/* LATER */
	default:	/* ignore unknown packet */
		break;
	}
}

/* init the vis server. this may only be called when if_list is already initialized
 * (e.g. bat0 is initialized, interfaces have been added) */
int vis_init(void) 
{
	vis_hash = hash_new(256, vis_info_cmp, vis_info_choose);
	if (vis_hash == NULL) {
		debug_log(LOG_TYPE_CRIT, "Can't initialize vis_hash\n");
		return(-1);
	}

	my_vis_info = kmalloc(1000, GFP_KERNEL);
	if (my_vis_info == NULL) {
		vis_quit();
		debug_log(LOG_TYPE_CRIT, "Can't initialize vis packet\n");
		return(-1);
	}
	/* prefill the vis info */
	my_vis_info->last_seen = jiffies - atomic_read(&vis_interval);
	my_vis_info->receive_from_list = NULL; /* LATER */
	my_vis_info->packet.packet_type = BAT_VIS;
	my_vis_info->packet.vis_type = VIS_TYPE_SERVER_SYNC;
	my_vis_info->packet.seqno = 0;
	my_vis_info->packet.entries = 0;

	spin_lock(&if_list_lock);
	memcpy(my_vis_info->packet.vis_orig, ((struct batman_if *)if_list.next)->net_dev->dev_addr, ETH_ALEN);
	memcpy(my_vis_info->packet.sender_orig, ((struct batman_if *)if_list.next)->net_dev->dev_addr, ETH_ALEN);
	spin_unlock(&if_list_lock);

	if (hash_add(vis_hash, my_vis_info) < 0) {
		debug_log(LOG_TYPE_CRIT, "Can't add own vis packet into hash\n");
		free_info(my_vis_info);	/* not in hash, need to remove it manually. */
		vis_quit();
		return(-1);
	}

	start_vis_timer();
	return(0);
}

/* free the info */
void free_info(void *data)
{
	struct vis_info *info = data;
	del_timer_sync(&info->vis_timer);
	kfree(info);
}

/* shutdown vis-server */
int vis_quit(void) 
{
	spin_lock(&vis_hash_lock);
	if (vis_hash != NULL)
		hash_delete(vis_hash, free_info);	/* properly remove, kill timers ... */
	vis_hash = NULL;
	my_vis_info = NULL;
	spin_unlock(&vis_hash_lock);
	return(0);
}

/* schedule own packet for (re)transmission */
void start_vis_timer(void)
{
	init_timer(&my_vis_info->vis_timer);

	my_vis_info->vis_timer.expires = jiffies + (atomic_read(&vis_interval)/1000) * HZ;
	my_vis_info->vis_timer.data = 0;
	my_vis_info->vis_timer.function = send_own_vis_packet;

	add_timer(&my_vis_info->vis_timer);
}

