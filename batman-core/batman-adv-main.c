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
#include "batman-adv-proc.h"
#include "batman-adv-log.h"
#include "batman-adv-routing.h"
#include "batman-adv-send.h"
#include "types.h"
#include "hash.h"



struct list_head if_list;
struct hashtable_t *orig_hash;

DEFINE_SPINLOCK(if_list_lock);
DEFINE_SPINLOCK(orig_hash_lock);

atomic_t originator_interval;
int16_t num_hna = 0;
int16_t num_ifs = 0;

struct task_struct *kthread_task = NULL;
struct timer_list purge_timer;

unsigned char broadcastAddr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};



int init_module(void)
{
	int retval;

	INIT_LIST_HEAD(&if_list);
	atomic_set(&originator_interval, 1000);

	if ((retval = setup_procfs()) < 0)
		return retval;

	orig_hash = hash_new(128, compare_orig, choose_orig);

	if (orig_hash == NULL) {
		cleanup_procfs();
		return -ENOMEM;
	}

	debug_log(LOG_TYPE_CRIT, "B.A.T.M.A.N. Advanced %s%s (compability version %i) loaded \n", SOURCE_VERSION, (strncmp(REVISION_VERSION, "0", 1) != 0 ? REVISION_VERSION : ""), COMPAT_VERSION);

	return 0;
}

void cleanup_module(void)
{
	shutdown_thread_timers();
	remove_interfaces();
	hash_delete(orig_hash, free_orig_node);
	cleanup_procfs();
}

void start_purge_timer(void)
{
	init_timer(&purge_timer);

	purge_timer.expires = jiffies + (1 * HZ); /* one second */
	purge_timer.data = 0;
	purge_timer.function = purge_orig;

	add_timer(&purge_timer);
}

void activate_thread_timers(void)
{
	struct list_head *list_pos;
	struct batman_if *batman_if = NULL;

	/* (re)activate all timers (if any) */
	list_for_each(list_pos, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);

		start_bcast_timer(batman_if);
	}

	/* (re)start kernel thread for packet processing */
	kthread_task = kthread_run(packet_recv_thread, NULL, "batman-adv");

	if (IS_ERR(kthread_task)) {
		debug_log(LOG_TYPE_CRIT, "Unable to start packet receive thread\n");
		kthread_task = NULL;
	}

	start_purge_timer();
}

void shutdown_thread_timers(void)
{
	struct list_head *list_pos;
	struct batman_if *batman_if = NULL;

	/* deactivate kernel thread for packet processing (if running) */
	if (kthread_task) {
		atomic_set(&exit_cond, 1);
		wake_up_interruptible(&thread_wait);
		kthread_stop(kthread_task);

		kthread_task = NULL;
	}

	/* deactivate all timers first to avoid race conditions */
	list_for_each(list_pos, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);

		del_timer_sync(&batman_if->bcast_timer);
	}

	if (!(list_empty(&if_list)))
		del_timer_sync(&purge_timer);
}

void remove_interfaces(void)
{
	struct list_head *list_pos, *list_pos_tmp;
	struct batman_if *batman_if = NULL;

	/* deactivate all interfaces */
	list_for_each_safe(list_pos, list_pos_tmp, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);

		debug_log(LOG_TYPE_NOTICE, "Deleting interface: %s\n", batman_if->net_dev->name);

		batman_if->raw_sock->sk->sk_data_ready = batman_if->raw_sock->sk->sk_user_data;

		list_del(list_pos);
		sock_release(batman_if->raw_sock);
		kfree(batman_if->pack_buff);
		kfree(batman_if);
	}
}

int add_interface(char *if_name, int if_num, struct net_device *net_dev)
{
	struct sockaddr_ll bind_addr;
	struct batman_if *batman_if;
	int retval;

	batman_if = kmalloc(sizeof(struct batman_if), GFP_KERNEL);

	if (!batman_if) {
		debug_log(LOG_TYPE_WARN, "Can't add interface (%s): out of memory\n", if_name);
		return -1;
	}

	if ((retval = sock_create_kern(PF_PACKET, SOCK_RAW, htons(ETH_P_BATMAN), &batman_if->raw_sock)) < 0) {
		debug_log(LOG_TYPE_WARN, "Can't create raw socket: %i\n", retval);
		goto batif_free;
	}

	bind_addr.sll_family = AF_PACKET;
	bind_addr.sll_ifindex = net_dev->ifindex;
	bind_addr.sll_protocol = 0;	/* is set by the kernel */

	if ((retval = kernel_bind(batman_if->raw_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr))) < 0) {
		debug_log(LOG_TYPE_WARN, "Can't create bind raw socket: %i\n", retval);
		goto sock_rel;
	}

	if ((if_num == 0) && (num_hna > 0))
		batman_if->pack_buff_len = sizeof(struct batman_packet) + num_hna * 6;
	else
		batman_if->pack_buff_len = sizeof(struct batman_packet);

	batman_if->pack_buff = kmalloc(batman_if->pack_buff_len, GFP_KERNEL);

	if (!batman_if->pack_buff) {
		debug_log(LOG_TYPE_WARN, "Can't add interface packet (%s): out of memory\n", if_name);
		goto sock_rel;
	}

	batman_if->raw_sock->sk->sk_user_data = batman_if->raw_sock->sk->sk_data_ready;
	batman_if->raw_sock->sk->sk_data_ready = batman_data_ready;

	batman_if->if_num = if_num;
	batman_if->net_dev = net_dev;
	addr_to_string(batman_if->addr_str, batman_if->net_dev->dev_addr);
	debug_log(LOG_TYPE_NOTICE, "Adding interface: %s\n", batman_if->net_dev->name);

	INIT_LIST_HEAD(&batman_if->list);
	list_add_tail(&batman_if->list, &if_list);

	((struct batman_packet *)(batman_if->pack_buff))->packet_type = BAT_PACKET;
	((struct batman_packet *)(batman_if->pack_buff))->version = COMPAT_VERSION;
	((struct batman_packet *)(batman_if->pack_buff))->flags = 0x00;
	((struct batman_packet *)(batman_if->pack_buff))->ttl = (batman_if->if_num > 0 ? 2 : TTL);
	((struct batman_packet *)(batman_if->pack_buff))->gwflags = 0;
	((struct batman_packet *)(batman_if->pack_buff))->tq = TQ_MAX_VALUE;
	memcpy(((struct batman_packet *)(batman_if->pack_buff))->orig, batman_if->net_dev->dev_addr, ETH_ALEN);
	memcpy(((struct batman_packet *)(batman_if->pack_buff))->old_orig, batman_if->net_dev->dev_addr, ETH_ALEN);

	batman_if->seqno = 1;
	batman_if->seqno_lock = __SPIN_LOCK_UNLOCKED(batman_if->seqno_lock);
	batman_if->bcast_seqno = 1;

	return 1;

sock_rel:
	sock_release(batman_if->raw_sock);
batif_free:
	kfree(batman_if);
	return -1;
}

void inc_module_count(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif
}

void dec_module_count(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif
}

int addr_to_string(char *buff, uint8_t *addr)
{
	return sprintf(buff, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

int compare_orig(void *data1, void *data2) {
	return (memcmp(data1, data2, ETH_ALEN) == 0 ? 1 : 0);
}

/* hashfunction to choose an entry in a hash table of given size */
/* hash algorithm from http://en.wikipedia.org/wiki/Hash_table */
int choose_orig(void *data, int32_t size)
{
	unsigned char *key= data;
	uint32_t hash = 0;
	size_t i;

	for (i = 0; i < 6; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return (hash%size);
}







/* int batman_core_attach(struct net_device *dev, u_int8_t *ie_buff, u_int8_t *ie_buff_len)
{
	struct list_head *list_pos;
	struct batman_if *batman_if = NULL;

	list_for_each(list_pos, &if_list) {

		batman_if = list_entry(list_pos, struct batman_if, list);

		if (batman_if->net_dev == dev)
			break;
		else
			batman_if = NULL;

	}

	if (batman_if == NULL) {

		printk( "B.A.T.M.A.N. Adv: attaching to %s\n", dev->name);

		batman_if = kmalloc(sizeof(struct batman_if), GFP_KERNEL);

		if (!batman_if)
			return -ENOMEM;

		batman_if->net_dev = dev;

		memcpy(batman_if->out.orig, batman_if->net_dev->dev_addr, 6);
		batman_if->out.packet_type = BAT_PACKET;
		batman_if->out.flags = 0x00;
		batman_if->out.ttl = TTL;
		batman_if->out.seqno = 1;
		batman_if->out.gwflags = 0;
		batman_if->out.version = COMPAT_VERSION;

		batman_if->bcast_seqno = 1;

		INIT_LIST_HEAD(&batman_if->list);
		list_add_tail(&batman_if->list, &if_list);

		memcpy(ie_buff, &batman_if->out, sizeof(struct batman_packet));
		*ie_buff_len = sizeof(struct batman_packet);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
		MOD_INC_USE_COUNT;
#else
		try_module_get(THIS_MODULE);
#endif
		return 0;

	}

	// return 1 to indicate that the interface is already attached
	return 1;
}
EXPORT_SYMBOL(batman_core_attach);

int batman_core_detach(struct net_device *dev)
{
	struct list_head *list_pos, *list_pos_tmp;
	struct batman_if *batman_if;

	list_for_each_safe(list_pos, list_pos_tmp, &if_list) {

		batman_if = list_entry(list_pos, struct batman_if, list);

		if (batman_if->net_dev == dev) {

			printk( "B.A.T.M.A.N. Adv: detaching from %s\n", batman_if->net_dev->name);

			list_del(list_pos);
			kfree(batman_if);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			MOD_DEC_USE_COUNT;
#else
			module_put(THIS_MODULE);
#endif
			return 0;

		}

	}

	// return 1 to indicate that the interface has not been attached yet
	return 1;
}
EXPORT_SYMBOL(batman_core_detach);

void batman_core_ogm_update(struct net_device *dev, u_int8_t *ie_buff, u_int8_t *ie_buff_len)
{
	struct list_head *list_pos;
	struct batman_if *batman_if = NULL;

	list_for_each(list_pos, &if_list) {

		batman_if = list_entry(list_pos, struct batman_if, list);

		if (batman_if->net_dev == dev)
			break;
		else
			batman_if = NULL;

	}

	if (batman_if != NULL) {

		printk( "B.A.T.M.A.N. Adv: updating ogm for %s\n", batman_if->net_dev->name);

		batman_if->out.seqno++;
		((struct batman_packet *)ie_buff)->seqno = batman_if->out.seqno;

	}
}
EXPORT_SYMBOL(batman_core_ogm_update); */



MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE(DRIVER_DEVICE);

