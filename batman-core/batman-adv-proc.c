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



static struct proc_dir_entry *proc_batman_dir = NULL, *proc_interface_file = NULL, *proc_orig_interval_file = NULL, *proc_originators_file = NULL, *proc_gateways_file = NULL;
static struct proc_dir_entry *proc_log_file = NULL;



void cleanup_procfs(void)
{
	if (proc_log_file)
		remove_proc_entry(PROC_FILE_LOG, proc_batman_dir);

	if (proc_gateways_file)
		remove_proc_entry(PROC_FILE_GATEWAYS, proc_batman_dir);

	if (proc_originators_file)
		remove_proc_entry(PROC_FILE_ORIGINATORS, proc_batman_dir);

	if (proc_orig_interval_file)
		remove_proc_entry(PROC_FILE_ORIG_INTERVAL, proc_batman_dir);

	if (proc_interface_file)
		remove_proc_entry(PROC_FILE_INTERFACES, proc_batman_dir);

	if (proc_batman_dir)
		remove_proc_entry(PROC_ROOT_DIR, proc_net);
}

int setup_procfs(void)
{
	proc_batman_dir = proc_mkdir(PROC_ROOT_DIR, proc_net);

	if (!proc_batman_dir) {
		printk("batman-adv: Registering the '/proc/net/%s' folder failed\n", PROC_ROOT_DIR);
		return -EFAULT;
	}

	proc_interface_file = create_proc_entry(PROC_FILE_INTERFACES, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_interface_file) {
		proc_interface_file->read_proc = proc_interfaces_read;
		proc_interface_file->write_proc = proc_interfaces_write;
		proc_interface_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_INTERFACES);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_orig_interval_file = create_proc_entry(PROC_FILE_ORIG_INTERVAL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_orig_interval_file) {
		proc_orig_interval_file->read_proc = proc_orig_interval_read;
		proc_orig_interval_file->write_proc = proc_orig_interval_write;
		proc_orig_interval_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_ORIG_INTERVAL);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_originators_file = create_proc_entry(PROC_FILE_ORIGINATORS, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_originators_file) {
		proc_originators_file->read_proc = proc_originators_read;
		proc_originators_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_ORIGINATORS);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_gateways_file = create_proc_entry(PROC_FILE_GATEWAYS, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_gateways_file) {
		proc_gateways_file->read_proc = proc_gateways_read;
		proc_gateways_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_GATEWAYS);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_log_file = create_proc_entry(PROC_FILE_LOG, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);
	if (proc_log_file) {
		proc_log_file->proc_fops = &proc_log_operations;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_FILE_LOG, PROC_FILE_GATEWAYS);
		cleanup_procfs();
		return -EFAULT;
	}

	return 0;
}

int proc_interfaces_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;
	struct list_head *list_pos;
	struct batman_if *batman_if;

	list_for_each(list_pos, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);

		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%s ", batman_if->net_dev->name);
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
	}

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "\n");
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}

int proc_interfaces_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	char *if_string, *colon_ptr = NULL, *cr_ptr = NULL;
	int not_copied = 0, retval, if_num = 0;
	struct net_device *net_dev;
	struct list_head *list_pos, *list_pos_tmp;
	struct batman_if *batman_if = NULL;
	struct sockaddr_ll bind_addr;

	if_string = kmalloc(count, GFP_KERNEL);

	if (!if_string)
		return -ENOMEM;

	if (count > IFNAMSIZ - 1) {
		debug_log(LOG_TYPE_WARN, "batman-adv: Can't add interface: device name is too long\n");
		goto end;
	}

	not_copied = copy_from_user(if_string, userbuffer, count);

	if ((colon_ptr = strchr(if_string, ':')) != NULL)
		*colon_ptr = 0;
	else if ((cr_ptr = strchr(if_string, '\n')) != NULL)
		*cr_ptr = 0;

	if (strlen(if_string) == 0) {

		/* deactivate all timers first to avoid race conditions */
		list_for_each(list_pos, &if_list) {
			batman_if = list_entry(list_pos, struct batman_if, list);

			del_timer_sync(&batman_if->bcast_timer);
		}

		/* deactivate all interfaces */
		list_for_each_safe(list_pos, list_pos_tmp, &if_list) {
			batman_if = list_entry(list_pos, struct batman_if, list);

			debug_log(LOG_TYPE_NOTICE, "batman-adv: Deleting interface: %s\n", batman_if->net_dev->name);

			list_del(list_pos);
			sock_release(batman_if->raw_sock);
			kfree(batman_if->pack_buff);
			kfree(batman_if);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			MOD_DEC_USE_COUNT;
#else
			module_put(THIS_MODULE);
#endif
		}

	} else {

		/* add interface */
		if ((net_dev = dev_get_by_name(if_string)) == NULL) {
			debug_log(LOG_TYPE_WARN, "batman-adv: Could not find interface: %s\n", if_string);
			goto end;
		}

		list_for_each(list_pos, &if_list) {
			batman_if = list_entry(list_pos, struct batman_if, list);

			if (batman_if->net_dev == net_dev)
				break;

			batman_if = NULL;
			if_num++;
		}

		if (batman_if != NULL) {

			debug_log(LOG_TYPE_WARN, "batman-adv: Given interface is already active: %s\n", if_string);
			goto end;

		} else {

			batman_if = kmalloc(sizeof(struct batman_if), GFP_KERNEL);

			if (!batman_if) {
				debug_log(LOG_TYPE_WARN, "batman-adv: Can't add interface (%s): out of memory\n", if_string);
				goto end;
			}

			if ((retval = sock_create_kern(PF_PACKET, SOCK_RAW, htons(ETH_P_BATMAN), &batman_if->raw_sock)) < 0) {
				debug_log(LOG_TYPE_WARN, "batman-adv: Can't create raw socket: %i\n", retval);
				kfree(batman_if);
				goto end;
			}

			bind_addr.sll_family = AF_PACKET;
			bind_addr.sll_ifindex = net_dev->ifindex;

			if ((retval = kernel_bind(batman_if->raw_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr))) < 0) {
				debug_log(LOG_TYPE_WARN, "batman-adv: Can't create bind raw socket: %i\n", retval);
				sock_release(batman_if->raw_sock);
				kfree(batman_if);
				goto end;
			}

			if ((if_num == 0) && (num_hna > 0))
				batman_if->pack_buff_len = sizeof(struct batman_packet) + num_hna * 6;
			else
				batman_if->pack_buff_len = sizeof(struct batman_packet);

			batman_if->pack_buff = kmalloc(batman_if->pack_buff_len, GFP_KERNEL);

			if (!batman_if->pack_buff) {
				debug_log(LOG_TYPE_WARN, "batman-adv: Can't add interface packet (%s): out of memory\n", if_string);
				sock_release(batman_if->raw_sock);
				kfree(batman_if);
				goto end;
			}

			batman_if->if_num = if_num;
			batman_if->net_dev = net_dev;
			debug_log(LOG_TYPE_NOTICE, "batman-adv: Adding interface: %s\n", batman_if->net_dev->name);

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
			batman_if->bcast_seqno = 1;

			init_timer(&batman_if->bcast_timer);

			batman_if->bcast_timer.expires = jiffies + (((originator_interval - JITTER + (random32() % 2*JITTER)) * HZ) / 1000);
			batman_if->bcast_timer.data = (unsigned long)batman_if;
			batman_if->bcast_timer.function = send_own_packet;

			add_timer(&batman_if->bcast_timer);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			MOD_INC_USE_COUNT;
#else
			try_module_get(THIS_MODULE);
#endif
		}

	}

end:
	kfree(if_string);
	return count;
}

int proc_orig_interval_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%i\n", originator_interval);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}

int proc_orig_interval_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	char *interval_string;
	int not_copied = 0;
	int16_t originator_interval_tmp;

	interval_string = kmalloc(count, GFP_KERNEL);

	if (!interval_string)
		return -ENOMEM;

	not_copied = copy_from_user(interval_string, userbuffer, count);

	originator_interval_tmp = simple_strtol(interval_string, NULL, 10);

	if (originator_interval_tmp < JITTER * 2) {
		debug_log(LOG_TYPE_WARN, "batman-adv: New originator interval too small: %i (min: %i)\n", originator_interval_tmp, JITTER * 2);
		goto end;
	}

	debug_log(LOG_TYPE_NOTICE, "batman-adv: Changing originator interval from: %i to: %i\n", originator_interval, originator_interval_tmp);

	originator_interval = originator_interval_tmp;

end:
	kfree(interval_string);
	return count;
}

int proc_originators_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;
// 	struct list_head *list_pos;
// 	struct batman_if *batman_if;
//
// TODO: loop through originators
// 	list_for_each(list_pos, &if_list) {
// 		batman_if = list_entry(list_pos, struct batman_if, list);
//
// 		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%s ", batman_if->net_dev->name);
// 		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
// 	}

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "\n");
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}

int proc_gateways_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;
// 	struct list_head *list_pos;
// 	struct batman_if *batman_if;
//
// TODO: loop through gateways
// 	list_for_each(list_pos, &if_list) {
// 		batman_if = list_entry(list_pos, struct batman_if, list);
	//
// 		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%s ", batman_if->net_dev->name);
// 		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
// 	}

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "\n");
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}


