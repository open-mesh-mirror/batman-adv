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



static struct proc_dir_entry *proc_batman_dir = NULL, *proc_interface_file = NULL, *proc_orig_interval_file = NULL;



void cleanup_procfs(void)
{
	if (proc_orig_interval_file)
		remove_proc_entry(PROC_FILE_ORIG_INTERVAL, proc_batman_dir);

	if (proc_interface_file)
		remove_proc_entry(PROC_FILE_INTERFACES, proc_batman_dir);

	if (proc_batman_dir)
		remove_proc_entry(PROC_ROOT_DIR, NULL);
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
		printk("batman-adv: Can't add interface: device name is too long\n");
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

			printk("batman-adv: Deleting interface: %s\n", batman_if->net_dev->name);

			list_del(list_pos);
			sock_release(batman_if->raw_sock);
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
			printk("batman-adv: Could not find interface: %s\n", if_string);
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

			printk("batman-adv: Given interface is already active: %s\n", if_string);
			goto end;

		} else {

			batman_if = kmalloc(sizeof(struct batman_if), GFP_KERNEL);

			if ((retval = sock_create_kern(PF_PACKET, SOCK_RAW, htons(ETH_P_BATMAN), &batman_if->raw_sock)) < 0) {

				printk("batman-adv: Can't create raw socket: %i\n", retval);
				kfree(batman_if);
				goto end;

			}

			bind_addr.sll_family   = AF_PACKET;
			bind_addr.sll_ifindex  = net_dev->ifindex;

			if ((retval = kernel_bind(batman_if->raw_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr))) < 0) {

				printk("batman-adv: Can't create bind raw socket: %i\n", retval);
				sock_release(batman_if->raw_sock);
				kfree(batman_if);
				goto end;

			}

			batman_if->if_num = if_num;
			batman_if->net_dev = net_dev;
			printk("batman-adv: Adding interface: %s\n", batman_if->net_dev->name);

			INIT_LIST_HEAD(&batman_if->list);
			list_add_tail(&batman_if->list, &if_list);

			batman_if->out.packet_type = BAT_PACKET;
			batman_if->out.version = COMPAT_VERSION;
			batman_if->out.flags = 0x00;
			batman_if->out.ttl = (batman_if->if_num > 0 ? 2 : TTL);
			batman_if->out.gwflags = 0;
			batman_if->out.tq = TQ_MAX_VALUE;
			batman_if->out.seqno = 1;
			memcpy(batman_if->out.orig, batman_if->net_dev->dev_addr, 6);
			memcpy(batman_if->out.old_orig, batman_if->net_dev->dev_addr, 6);

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

	if ( originator_interval_tmp < JITTER * 2 ) {
		printk("batman-adv: New originator interval too small: %i (min: %i)\n", originator_interval_tmp, JITTER * 2);
		goto end;
	}

	originator_interval = originator_interval_tmp;

end:
	kfree(interval_string);
	return count;
}


