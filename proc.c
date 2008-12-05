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
#include "proc.h"
#include "log.h"
#include "routing.h"
#include "translation-table.h"
#include "hard-interface.h"
#include "vis.h"
#include "types.h"
#include "hash.h"



static struct proc_dir_entry *proc_batman_dir = NULL, *proc_interface_file = NULL, *proc_orig_interval_file = NULL, *proc_originators_file = NULL;
static struct proc_dir_entry *proc_log_file = NULL, *proc_log_level_file = NULL, *proc_transtable_local_file = NULL, *proc_transtable_global_file = NULL, *proc_vis_file = NULL;

static const struct file_operations proc_vis_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_vis_open,
	.read		= seq_read,
	.write		= proc_vis_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations proc_originators_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_originators_open,
	.read		= seq_read,
	.write		= proc_dummy_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations proc_transtable_local_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_transtable_local_open,
	.read		= seq_read,
	.write		= proc_dummy_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations proc_transtable_global_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_transtable_global_open,
	.read		= seq_read,
	.write		= proc_dummy_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations proc_log_level_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_log_level_open,
	.read		= seq_read,
	.write		= proc_log_level_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations proc_interfaces_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_interfaces_open,
	.read		= seq_read,
	.write		= proc_interfaces_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations proc_orig_interval_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_orig_interval_open,
	.read		= seq_read,
	.write		= proc_orig_interval_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};



void cleanup_procfs(void)
{
	if (proc_transtable_global_file)
		remove_proc_entry(PROC_FILE_TRANSTABLE_GLOBAL, proc_batman_dir);

	if (proc_transtable_local_file)
		remove_proc_entry(PROC_FILE_TRANSTABLE_LOCAL, proc_batman_dir);

	if (proc_log_file)
		remove_proc_entry(PROC_FILE_LOG, proc_batman_dir);

	if (proc_log_level_file)
		remove_proc_entry(PROC_FILE_LOG_LEVEL, proc_batman_dir);

	if (proc_originators_file)
		remove_proc_entry(PROC_FILE_ORIGINATORS, proc_batman_dir);

	if (proc_orig_interval_file)
		remove_proc_entry(PROC_FILE_ORIG_INTERVAL, proc_batman_dir);

	if (proc_interface_file)
		remove_proc_entry(PROC_FILE_INTERFACES, proc_batman_dir);

	if (proc_vis_file)
		remove_proc_entry(PROC_FILE_VIS, proc_batman_dir);



	if (proc_batman_dir)
#ifdef __NET_NET_NAMESPACE_H
		remove_proc_entry(PROC_ROOT_DIR, init_net.proc_net);
#else
		remove_proc_entry(PROC_ROOT_DIR, proc_net);
#endif
}

int setup_procfs(void)
{
#ifdef __NET_NET_NAMESPACE_H
	proc_batman_dir = proc_mkdir(PROC_ROOT_DIR, init_net.proc_net);
#else
	proc_batman_dir = proc_mkdir(PROC_ROOT_DIR, proc_net);
#endif

	if (!proc_batman_dir) {
		printk("batman-adv: Registering the '/proc/net/%s' folder failed\n", PROC_ROOT_DIR);
		return -EFAULT;
	}

	proc_interface_file = create_proc_entry(PROC_FILE_INTERFACES, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_interface_file) {
		proc_interface_file->proc_fops = &proc_interfaces_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_INTERFACES);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_orig_interval_file = create_proc_entry(PROC_FILE_ORIG_INTERVAL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_orig_interval_file) {
		proc_orig_interval_file->proc_fops = &proc_orig_interval_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_ORIG_INTERVAL);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_log_level_file = create_proc_entry(PROC_FILE_LOG_LEVEL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_log_level_file) {

		proc_log_level_file->proc_fops = &proc_log_level_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_LOG_LEVEL);
		cleanup_procfs();
		return -EFAULT;
	}


	proc_originators_file = create_proc_entry(PROC_FILE_ORIGINATORS, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_originators_file) {
		proc_originators_file->proc_fops = &proc_originators_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_ORIGINATORS);
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

	proc_transtable_local_file = create_proc_entry(PROC_FILE_TRANSTABLE_LOCAL, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_transtable_local_file) {
		proc_transtable_local_file->proc_fops = &proc_transtable_local_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_TRANSTABLE_LOCAL);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_transtable_global_file = create_proc_entry(PROC_FILE_TRANSTABLE_GLOBAL, S_IRUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_transtable_global_file) {
		proc_transtable_global_file->proc_fops = &proc_transtable_global_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_TRANSTABLE_GLOBAL);
		cleanup_procfs();
		return -EFAULT;
	}

	proc_vis_file = create_proc_entry(PROC_FILE_VIS, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, proc_batman_dir);

	if (proc_vis_file) {
		proc_vis_file->proc_fops = &proc_vis_fops;
	} else {
		printk("batman-adv: Registering the '/proc/net/%s/%s' file failed\n", PROC_ROOT_DIR, PROC_FILE_VIS);
		cleanup_procfs();
		return -EFAULT;
	}


	return 0;
}

int proc_interfaces_read(struct seq_file *seq, void *offset)
{
	struct batman_if *batman_if;

	rcu_read_lock();
	list_for_each_entry_rcu(batman_if, &if_list, list) {
		seq_printf(seq, "%s ", batman_if->dev);
	}
	rcu_read_unlock();

	seq_printf(seq, "\n");
	return 0;
}

int proc_interfaces_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_interfaces_read, NULL);
}

ssize_t proc_interfaces_write(struct file *instance, const char __user *userbuffer, size_t count, loff_t *data)
{
	char *if_string, *colon_ptr = NULL, *cr_ptr = NULL;
	int not_copied = 0, if_num = 0;
	struct batman_if *batman_if = NULL;

	if_string = kmalloc(count, GFP_KERNEL);

	if (!if_string)
		return -ENOMEM;

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

	if (strlen(if_string) == 0) {

		shutdown_module();

		spin_lock(&orig_hash_lock);
		hash_delete(orig_hash, free_orig_node);
		orig_hash = hash_new(128, compare_orig, choose_orig);
		spin_unlock(&orig_hash_lock);

		num_ifs = 0;
		goto end;

	} else {

		/* add interface */
		rcu_read_lock();
		list_for_each_entry_rcu(batman_if, &if_list, list) {

			if (strncmp(batman_if->dev, if_string, count) == 0) {
				debug_log(LOG_TYPE_WARN, "Given interface is already active: %s\n", if_string);
				rcu_read_unlock();
				goto end;

			}

			if_num++;
		}
		rcu_read_unlock();

		if (module_state == MODULE_INACTIVE)
			module_state = MODULE_WAITING;

		hardif_add_interface(if_string, if_num);

		if (module_state == MODULE_WAITING) {
			if (hardif_get_active_if_num() > 0)
				activate_module();
			else 
				debug_log(LOG_TYPE_WARN, "Can't activate module: the primary interface is not active\n");
		}
	}

	rcu_read_lock();
	if (list_empty(&if_list)) {
		rcu_read_unlock();
		goto end;
	}
	rcu_read_unlock();

	num_ifs = if_num + 1;
	return count;

end:
	kfree(if_string);
	return count;
}

int proc_orig_interval_read(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%i\n", atomic_read(&originator_interval));

	return 0;
}

ssize_t proc_orig_interval_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	char *interval_string;
	int not_copied = 0;
	int16_t originator_interval_tmp;

	interval_string = kmalloc(count, GFP_KERNEL);

	if (!interval_string)
		return -ENOMEM;

	not_copied = copy_from_user(interval_string, buffer, count);
	interval_string[count - not_copied - 1] = 0;

	originator_interval_tmp = simple_strtol(interval_string, NULL, 10);

	if (originator_interval_tmp <= JITTER * 2) {
		debug_log(LOG_TYPE_WARN, "New originator interval too small: %i (min: %i)\n", originator_interval_tmp, JITTER * 2);
		goto end;
	}

	debug_log(LOG_TYPE_NOTICE, "Changing originator interval from: %i to: %i\n", atomic_read(&originator_interval), originator_interval_tmp);

	atomic_set(&originator_interval, originator_interval_tmp);

end:
	kfree(interval_string);
	return count;
}

int proc_orig_interval_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_orig_interval_read, NULL);
}

int proc_originators_read(struct seq_file *seq, void *offset)
{
	struct hash_it_t *hashit = NULL;
	struct orig_node *orig_node;
	struct neigh_node *neigh_node;
	int batman_count = 0;
	char orig_str[ETH_STR_LEN], router_str[ETH_STR_LEN];

	rcu_read_lock();
	if (list_empty(&if_list)) {
		rcu_read_unlock();
		seq_printf(seq, "BATMAN disabled - please specify interfaces to enable it \n");
		goto end;
	}

	seq_printf(seq, "  %-14s (%s/%i) %17s [%10s]: %20s ... [B.A.T.M.A.N. Adv %s%s, MainIF/MAC: %s/%s] \n", "Originator", "#", TQ_MAX_VALUE, "Nexthop", "outgoingIF", "Potential nexthops", SOURCE_VERSION, (strlen(REVISION_VERSION) > 3 ? REVISION_VERSION : ""), ((struct batman_if *)if_list.next)->dev, ((struct batman_if *)if_list.next)->addr_str);

	rcu_read_unlock();
	spin_lock(&orig_hash_lock);

	while (NULL != (hashit = hash_iterate( orig_hash, hashit))) {

		orig_node = hashit->bucket->data;

		if (orig_node->router == NULL)
			continue;

		if (orig_node->router->tq_avg == 0)
			continue;

		batman_count++;

		addr_to_string(orig_str, orig_node->orig);
		addr_to_string(router_str, orig_node->router->addr);

		seq_printf(seq, "%-17s  (%3i) %17s [%10s]:", orig_str, orig_node->router->tq_avg, router_str, orig_node->router->if_incoming->dev);

		list_for_each_entry(neigh_node, &orig_node->neigh_list, list) {
			addr_to_string(orig_str, neigh_node->addr);
			seq_printf(seq, " %17s (%3i)", orig_str, neigh_node->tq_avg);
		}

		seq_printf(seq, "\n");

	}

	spin_unlock(&orig_hash_lock);

	if (batman_count == 0)
		seq_printf(seq, "No batman nodes in range ... \n");

end:
	return 0;
}

int proc_originators_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_originators_read, NULL);
}

int proc_log_level_read(struct seq_file *seq, void *offset)
{

	seq_printf(seq, "[x] %s (%d)\n", LOG_TYPE_CRIT_NAME, LOG_TYPE_CRIT);
	seq_printf(seq, "[%c] %s (%d)\n", (LOG_TYPE_WARN & log_level) ? 'x' : ' ', LOG_TYPE_WARN_NAME, LOG_TYPE_WARN);
	seq_printf(seq, "[%c] %s (%d)\n", (LOG_TYPE_NOTICE & log_level) ? 'x' : ' ', LOG_TYPE_NOTICE_NAME, LOG_TYPE_NOTICE);
	seq_printf(seq, "[%c] %s (%d)\n", (LOG_TYPE_BATMAN & log_level) ? 'x' : ' ', LOG_TYPE_BATMAN_NAME, LOG_TYPE_BATMAN);
	seq_printf(seq, "[%c] %s (%d)\n", (LOG_TYPE_ROUTES & log_level) ? 'x' : ' ', LOG_TYPE_ROUTES_NAME, LOG_TYPE_ROUTES);

	return 0;
}

int proc_log_level_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_log_level_read, NULL);
}


ssize_t proc_log_level_write(struct file *instance, const char __user *userbuffer, size_t count, loff_t *data)
{
	char *log_level_string, *tokptr, *cp;
	int finished, not_copied = 0;
	uint8_t log_level_tmp = 0;

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
					log_level_tmp |= LOG_TYPE_WARN;
				if (strcmp(tokptr, LOG_TYPE_NOTICE_NAME) == 0)
					log_level_tmp |= LOG_TYPE_NOTICE;
				if (strcmp(tokptr, LOG_TYPE_BATMAN_NAME) == 0)
					log_level_tmp |= LOG_TYPE_BATMAN;
				if (strcmp(tokptr, LOG_TYPE_ROUTES_NAME) == 0)
					log_level_tmp |= LOG_TYPE_ROUTES;
				tokptr = cp + 1;
				break;
			default:
				;
			}
		}
	}

	debug_log(LOG_TYPE_CRIT, "Changing log_level from: %i to: %i\n", log_level, log_level_tmp);
	log_level = log_level_tmp;

	kfree(log_level_string);
	return count;
}

int proc_transtable_local_read(struct seq_file *seq, void *offset)
{
	char *buf;

	buf = kmalloc(4096, GFP_KERNEL);
	if (!buf)
		return 0;

	rcu_read_lock();
	if (list_empty(&if_list)) {
		rcu_read_unlock();
		seq_printf(seq, "BATMAN disabled - please specify interfaces to enable it \n");
		goto end;
	}

	rcu_read_unlock();

	seq_printf(seq, "Locally retrieved addresses (from %s) announced via HNA:\n", soft_device->name);

	hna_local_fill_buffer_text(buf, 4096);
	seq_printf(seq, "%s", buf);

end:
	kfree(buf);
	return 0;
}

int proc_transtable_local_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_transtable_local_read, NULL);
}


int proc_transtable_global_read(struct seq_file *seq, void *offset)
{
	char *buf;

	buf = kmalloc(4096, GFP_KERNEL);
	if (!buf)
		return 0;

	rcu_read_lock();
	if (list_empty(&if_list)) {
		rcu_read_unlock();
		seq_printf(seq, "BATMAN disabled - please specify interfaces to enable it \n");
		goto end;
	}
	rcu_read_unlock();


	seq_printf(seq, "Globally announced HNAs received via the mesh (translation table):\n");

	hna_global_fill_buffer_text(buf, 4096);
	seq_printf(seq, "%s", buf);

end:
	kfree(buf);
	return 0;
}
int proc_transtable_global_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_transtable_global_read, NULL);
}


int proc_vis_read(struct seq_file *seq, void *offset)
{
	struct hash_it_t *hashit = NULL;
	struct vis_info *info;
	struct vis_info_entry *entries;
	char from[40], to[40];
	int i, int_part, frac_part;


	rcu_read_lock();
	if (list_empty(&if_list) || (!is_vis_server())) {
		rcu_read_unlock();
		seq_printf(seq, "digraph {\n}\n" );
		goto end;
	}

	rcu_read_unlock();

	seq_printf(seq, "digraph {\n" );

	spin_lock(&vis_hash_lock);
	while (NULL != (hashit = hash_iterate(vis_hash, hashit))) {
		info = hashit->bucket->data;
		entries = (struct vis_info_entry *)((char *)info + sizeof(struct vis_info));
		addr_to_string(from, info->packet.vis_orig);
		for (i = 0; i < info->packet.entries; i++) {
			addr_to_string(to, entries[i].dest);
			if (entries[i].quality == 0)
				seq_printf(seq, "\t\"%s\" -> \"%s\" [label=\"HNA\"]\n", from, to);
			else {
				/* kernel has no printf-support for %f? it'd be better to return this in float. */
				int_part = 255/entries[i].quality;
				frac_part = 1000 * 255/entries[i].quality - int_part * 1000;
				seq_printf(seq, "\t\"%s\" -> \"%s\" [label=\"%d.%d\"]\n", from, to, int_part, frac_part);
			}
		}
	}

	spin_unlock(&vis_hash_lock);
	seq_printf(seq, "}\n");
end:
	return 0;
}

/* setting the mode of the vis server by the user */
ssize_t proc_vis_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	char *vis_mode_string;
	int not_copied = 0;

	vis_mode_string = kmalloc(count, GFP_KERNEL);

	if (!vis_mode_string)
		return -ENOMEM;

	not_copied = copy_from_user(vis_mode_string, buffer, count);
	vis_mode_string[count - not_copied - 1] = 0;

	if (strcmp(vis_mode_string, "client") == 0) {
		debug_log(LOG_TYPE_NOTICE, "Setting VIS mode to client\n");
		vis_set_mode(VIS_TYPE_CLIENT_UPDATE);
	}
	else if (strcmp(vis_mode_string, "server") == 0) {
		debug_log(LOG_TYPE_NOTICE, "Setting VIS mode to server\n");
		vis_set_mode(VIS_TYPE_SERVER_SYNC);
	} else 
		debug_log(LOG_TYPE_WARN, "unknown vis-server mode: %s\n", vis_mode_string);
	



	kfree(vis_mode_string);
	return count;
}

int proc_vis_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_vis_read, NULL);
}


/* satisfying different prototypes ... */
ssize_t proc_dummy_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	return count;
}

