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
#include "types.h"
#include "hash.h"



static struct proc_dir_entry *proc_batman_dir = NULL, *proc_interface_file = NULL, *proc_orig_interval_file = NULL, *proc_originators_file = NULL, *proc_gateways_file = NULL;
static struct proc_dir_entry *proc_log_file = NULL, *proc_log_level_file = NULL;



void cleanup_procfs(void)
{
	if (proc_log_file)
		remove_proc_entry(PROC_FILE_LOG, proc_batman_dir);

	if (proc_log_level_file)
		remove_proc_entry(PROC_FILE_LOG_LEVEL, proc_batman_dir);

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

	proc_log_level_file = create_proc_entry(PROC_FILE_LOG_LEVEL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_log_level_file) {
		proc_log_level_file->read_proc = proc_log_level_read;
		proc_log_level_file->write_proc = proc_log_level_write;
		proc_log_level_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_LOG_LEVEL);
		cleanup_procfs();
		return -EFAULT;
	}


	proc_originators_file = create_proc_entry(PROC_FILE_ORIGINATORS, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_originators_file) {
		proc_originators_file->read_proc = proc_originators_read;
		proc_originators_file->write_proc = proc_originators_write;
		proc_originators_file->data = NULL;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_ORIGINATORS);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_gateways_file = create_proc_entry(PROC_FILE_GATEWAYS, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_gateways_file) {
		proc_gateways_file->read_proc = proc_gateways_read;
		proc_gateways_file->write_proc = proc_gateways_write;
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

	spin_lock(&if_list_lock);

	list_for_each(list_pos, &if_list) {
		batman_if = list_entry(list_pos, struct batman_if, list);

		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%s ", batman_if->net_dev->name);
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
	}

	spin_unlock(&if_list_lock);

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "\n");
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}

int proc_interfaces_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	char *if_string, *colon_ptr = NULL, *cr_ptr = NULL;
	int not_copied = 0, if_num;
	void *data_ptr;
	struct net_device *net_dev;
	struct list_head *list_pos;
	struct batman_if *batman_if = NULL;
	struct hash_it_t *hashit = NULL;
	struct orig_node *orig_node;

	if_string = kmalloc(count, GFP_KERNEL);

	if (!if_string)
		return -ENOMEM;

	spin_lock(&if_list_lock);

	if (count > IFNAMSIZ - 1) {
		debug_log(LOG_TYPE_WARN, "Can't add interface: device name is too long\n");
		goto end;
	}

	not_copied = copy_from_user(if_string, userbuffer, count);
	if_string[count - not_copied - 1] = 0;

	if ((colon_ptr = strchr(if_string, ':')) != NULL)
		*colon_ptr = 0;
	else if ((cr_ptr = strchr(if_string, '\n')) != NULL)
		*cr_ptr = 0;

	shutdown_module();

	num_ifs = if_num = 0;

	if (strlen(if_string) == 0) {

		remove_interfaces();
		hash_delete(orig_hash, free_orig_node);
		orig_hash = hash_new(128, compare_orig, choose_orig);

	} else {

		/* add interface */
		if ((net_dev = dev_get_by_name(if_string)) == NULL) {
			debug_log(LOG_TYPE_WARN, "Could not find interface: %s\n", if_string);
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

			debug_log(LOG_TYPE_WARN, "Given interface is already active: %s\n", if_string);
			goto end;

		} else {

			if (add_interface(if_string, if_num, net_dev)) {

				/* resize all orig nodes because orig_node->bcast_own(_sum) depend on if_num */
				spin_lock(&orig_hash_lock);

				while (NULL != (hashit = hash_iterate(orig_hash, hashit))) {
					orig_node = hashit->bucket->data;

					data_ptr = kmalloc((if_num + 1) * sizeof(TYPE_OF_WORD) * NUM_WORDS, GFP_KERNEL);
					memcpy(data_ptr, orig_node->bcast_own, if_num * sizeof(TYPE_OF_WORD) * NUM_WORDS);
					kfree(orig_node->bcast_own);
					orig_node->bcast_own = data_ptr;

					data_ptr = kmalloc((if_num + 1) * sizeof(uint8_t), GFP_KERNEL);
					memcpy(data_ptr, orig_node->bcast_own_sum, if_num * sizeof(uint8_t));
					kfree(orig_node->bcast_own_sum);
					orig_node->bcast_own_sum = data_ptr;
				}

				spin_unlock(&orig_hash_lock);
			}
		}
	}

	if (list_empty(&if_list))
		goto end;

	num_ifs = if_num + 1;

	activate_module();

end:
	spin_unlock(&if_list_lock);
	kfree(if_string);
	return count;
}

int proc_orig_interval_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%i\n", atomic_read(&originator_interval));
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
	interval_string[count - not_copied - 1] = 0;

	originator_interval_tmp = simple_strtol(interval_string, NULL, 10);

	if (originator_interval_tmp < JITTER * 2) {
		debug_log(LOG_TYPE_WARN, "New originator interval too small: %i (min: %i)\n", originator_interval_tmp, JITTER * 2);
		goto end;
	}

	debug_log(LOG_TYPE_NOTICE, "Changing originator interval from: %i to: %i\n", atomic_read(&originator_interval), originator_interval_tmp);

	atomic_set(&originator_interval, originator_interval_tmp);

end:
	kfree(interval_string);
	return count;
}

int proc_originators_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	struct hash_it_t *hashit = NULL;
	struct list_head *list_pos;
	struct orig_node *orig_node;
	struct neigh_node *neigh_node;
	int total_bytes = 0, bytes_written = 0, batman_count = 0;
	char orig_str[ETH_STR_LEN], router_str[ETH_STR_LEN];

	spin_lock(&if_list_lock);
	if (list_empty(&if_list)) {
		spin_unlock(&if_list_lock);
		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "BATMAN disabled - please specify interfaces to enable it \n");
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
		goto end;
	}

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "  %-14s (%s/%i) %17s [%10s]: %20s ... [B.A.T.M.A.N. Adv %s%s, MainIF/MAC: %s/%s] \n", "Originator", "#", TQ_MAX_VALUE, "Nexthop", "outgoingIF", "Potential nexthops", SOURCE_VERSION, (strncmp( REVISION_VERSION, "0", 1 ) != 0 ? REVISION_VERSION : ""), ((struct batman_if *)if_list.next)->net_dev->name, ((struct batman_if *)if_list.next)->addr_str);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	spin_unlock(&if_list_lock);
	spin_lock(&orig_hash_lock);

	while (NULL != (hashit = hash_iterate( orig_hash, hashit))) {

		orig_node = hashit->bucket->data;

		if (orig_node->router == NULL)
			continue;

		batman_count++;

		addr_to_string(orig_str, orig_node->orig);
		addr_to_string(router_str, orig_node->router->addr);

		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "%-17s  (%3i) %17s [%10s]:", orig_str, orig_node->router->tq_avg, router_str, orig_node->router->if_incoming->net_dev->name);
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

		list_for_each(list_pos, &orig_node->neigh_list) {
			neigh_node = list_entry(list_pos, struct neigh_node, list);

			addr_to_string(orig_str, neigh_node->addr);

			bytes_written = snprintf(buf + total_bytes, (size - total_bytes), " %17s (%3i)", orig_str, neigh_node->tq_avg);
			total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
		}

		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "\n");
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	}

	spin_unlock(&orig_hash_lock);

	if (batman_count == 0) {
		bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "No batman nodes in range ... \n");
		total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);
	}

end:
	*eof = 1;
	return total_bytes;
}

int proc_originators_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	return count;
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

int proc_gateways_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	return count;
}

int proc_log_level_read(char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int total_bytes = 0, bytes_written = 0;

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "[x] %s (%d)\n",
				LOG_TYPE_CRIT_NAME, LOG_TYPE_CRIT);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "[%c] %s (%d)\n",
				test_bit(LOG_TYPE_WARN, &log_level) ? 'x' : ' ', LOG_TYPE_WARN_NAME, LOG_TYPE_WARN_POW2);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "[%c] %s (%d)\n",
				 test_bit(LOG_TYPE_NOTICE, &log_level) ? 'x' : ' ', LOG_TYPE_NOTICE_NAME, LOG_TYPE_NOTICE_POW2);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "[%c] %s (%d)\n",
				 test_bit(LOG_TYPE_BATMAN, &log_level) ? 'x' : ' ', LOG_TYPE_BATMAN_NAME, LOG_TYPE_BATMAN_POW2);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	bytes_written = snprintf(buf + total_bytes, (size - total_bytes), "[%c] %s (%d)\n",
				 test_bit(LOG_TYPE_ROUTES, &log_level) ? 'x' : ' ', LOG_TYPE_ROUTES_NAME, LOG_TYPE_ROUTES_POW2);
	total_bytes += (bytes_written > (size - total_bytes) ? size - total_bytes : bytes_written);

	*eof = 1;
	return total_bytes;
}

int proc_log_level_write(struct file *instance, const char __user *userbuffer, unsigned long count, void *data)
{
	char *log_level_string, *tokptr, *cp;
	int finished, not_copied = 0;
	unsigned long log_level_tmp = 0;

	log_level_string = kmalloc(count, GFP_KERNEL);

	if (!log_level_string)
		return -ENOMEM;

	not_copied = copy_from_user(log_level_string, userbuffer, count);
	log_level_string[count - not_copied - 1] = 0;

	log_level_tmp = simple_strtol(log_level_string, &cp, 10);

	if (cp == log_level_string) {
		/* was not (beginning with) a number, doing textual parsing */
		log_level_tmp = 0;
		tokptr = log_level_string;

		for (cp = log_level_string, finished = 0; !finished; cp++) {
			switch (*cp) {
			case 0:
				finished = 1;
			case ' ':
			case '\n':
			case '\t':
				*cp = 0;
				/* compare */
				if (strcmp(tokptr, LOG_TYPE_WARN_NAME) == 0)
					log_level_tmp |= LOG_TYPE_WARN_POW2;
				if (strcmp(tokptr, LOG_TYPE_NOTICE_NAME) == 0)
					log_level_tmp |= LOG_TYPE_NOTICE_POW2;
				if (strcmp(tokptr, LOG_TYPE_BATMAN_NAME) == 0)
					log_level_tmp |= LOG_TYPE_BATMAN_POW2;
				if (strcmp(tokptr, LOG_TYPE_ROUTES_NAME) == 0)
					log_level_tmp |= LOG_TYPE_ROUTES_POW2;
				tokptr = cp + 1;
				break;
			default:
				;
			}
		}
	}

	debug_log(LOG_TYPE_CRIT, "Changing log_level from: %i to: %i\n", log_level, log_level_tmp);

	if (LOG_TYPE_WARN_POW2 & log_level_tmp)
		set_bit(LOG_TYPE_WARN, &log_level);
	else
		clear_bit(LOG_TYPE_WARN, &log_level);

	if (LOG_TYPE_NOTICE_POW2 & log_level_tmp)
		set_bit(LOG_TYPE_NOTICE, &log_level);
	else
		clear_bit(LOG_TYPE_NOTICE, &log_level);

	if (LOG_TYPE_BATMAN_POW2 & log_level_tmp)
		set_bit(LOG_TYPE_BATMAN, &log_level);
	else
		clear_bit(LOG_TYPE_BATMAN, &log_level);

	if (LOG_TYPE_ROUTES_POW2 & log_level_tmp)
		set_bit(LOG_TYPE_ROUTES, &log_level);
	else
		clear_bit(LOG_TYPE_ROUTES, &log_level);

	kfree(log_level_string);
	return count;
}

