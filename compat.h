/*
 * Copyright (C) 2007-2010 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
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
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include "bat_sysfs.h"		/* struct bat_attribute */

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)

#define skb_set_network_header(_skb, _offset) \
	do { (_skb)->nh.raw = (_skb)->data + (_offset); } while (0)

#define skb_reset_mac_header(_skb) \
	do { (_skb)->mac.raw = (_skb)->data; } while (0)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define skb_mac_header(_skb) \
    ((_skb)->mac.raw)

#define skb_network_header(_skb) \
    ((_skb)->nh.raw)

#define skb_mac_header(_skb) \
    ((_skb)->mac.raw)

#endif /* < KERNEL_VERSION(2,6,22) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 23)

#define transtable_local_read(kobj, attr, buff, off, count) \
	transtable_local_read(kobj, buff, off, count)
#define transtable_global_read(kobj, attr, buff, off, count) \
	transtable_global_read(kobj, buff, off, count)
#define originators_read(kobj, attr, buff, off, count) \
	originators_read(kobj, buff, off, count)
#define gateways_read(kobj, attr, buff, off, count) \
	gateways_read(kobj, buff, off, count)
#define vis_data_read(kobj, attr, buff, off, count) \
	vis_data_read(kobj, buff, off, count)

static inline int skb_clone_writable(struct sk_buff *skb, unsigned int len)
{
	/* skb->hdr_len not available, just "not writable" to enforce a copy */
	return 0;
}

#define cancel_delayed_work_sync(wq) cancel_delayed_work(wq)

#endif /* < KERNEL_VERSION(2, 6, 23) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

#endif /* < KERNEL_VERSION(2, 6, 24) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)

#define strict_strtoul(cp, base, res) \
	({ \
	int ret = 0; \
	char *endp; \
	*res = simple_strtoul(cp, &endp, base); \
	if (cp == endp) \
		ret = -EINVAL; \
	ret; \
})

#define to_battr(a) container_of(a, struct bat_attribute, attr)

static inline ssize_t bat_wrapper_show(struct kobject *kobj,
				   struct attribute *attr, char *buf)
{
	struct bat_attribute *bat_attr = to_battr(attr);

	if (bat_attr->show)
		return bat_attr->show(kobj, attr, buf);

	return -EIO;
}

static inline ssize_t bat_wrapper_store(struct kobject *kobj,
				    struct attribute *attr,
				    const char *buf, size_t count)
{
	struct bat_attribute *bat_attr = to_battr(attr);

	if (bat_attr->store)
		return bat_attr->store(kobj, attr, (char *)buf, count);

	return -EIO;
}

static struct sysfs_ops bat_wrapper_ops = {
	.show   = bat_wrapper_show,
	.store  = bat_wrapper_store,
};

static struct kobj_type ktype_bat_wrapper = {
	.sysfs_ops      = &bat_wrapper_ops,
};

static inline struct kobject *kobject_create_and_add(const char *name,
						     struct kobject *parent)
{
	struct kobject *kobj;
	int err;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (!kobj)
		return NULL;

	kobject_set_name(kobj, name);
	kobj->ktype = &ktype_bat_wrapper;
	kobj->kset = NULL;
	kobj->parent = parent;

	err = kobject_register(kobj);
	if (err) {
		kobject_put(kobj);
		return NULL;
	}

	return kobj;
}

#endif /* < KERNEL_VERSION(2, 6, 25) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)

static const char hex_asc[] = "0123456789abcdef";
#define hex_asc_lo(x)   hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)   hex_asc[((x) & 0xf0) >> 4]
static inline char *pack_hex_byte(char *buf, u8 byte)
{
    *buf++ = hex_asc_hi(byte);
    *buf++ = hex_asc_lo(byte);
    return buf;
}

#endif /* < KERNEL_VERSION(2, 6, 26) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)

#ifndef dereference_function_descriptor
#define dereference_function_descriptor(p) (p)
#endif

#include <linux/debugfs.h>

static inline void debugfs_remove_recursive(struct dentry *dentry)
{
	struct dentry *child;
	struct dentry *parent;

	if (!dentry)
		return;

	parent = dentry->d_parent;
	if (!parent || !parent->d_inode)
		return;

	parent = dentry;

	while (1) {
		/*
		 * When all dentries under "parent" has been removed,
		 * walk up the tree until we reach our starting point.
		 */
		if (list_empty(&parent->d_subdirs)) {
			if (parent == dentry)
				break;
			parent = parent->d_parent;
		}
		child = list_entry(parent->d_subdirs.next, struct dentry,
				d_u.d_child);
next_sibling:

		/*
		 * If "child" isn't empty, walk down the tree and
		 * remove all its descendants first.
		 */
		if (!list_empty(&child->d_subdirs)) {
			parent = child;
			continue;
		}
		debugfs_remove(child);
		if (parent->d_subdirs.next == &child->d_u.d_child) {
			/*
			 * Try the next sibling.
			 */
			if (child->d_u.d_child.next != &parent->d_subdirs) {
				child = list_entry(child->d_u.d_child.next,
						   struct dentry,
						   d_u.d_child);
				goto next_sibling;
			}
			break;
		}
	}

	debugfs_remove(dentry);
}

#endif /* < KERNEL_VERSION(2, 6, 27) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)

asmlinkage int bat_printk(const char *fmt, ...);
#define printk bat_printk

static inline struct net_device_stats *dev_get_stats(struct net_device *dev)
{
	if (dev->get_stats)
		return dev->get_stats(dev);
	else
		return NULL;
}

#endif /* < KERNEL_VERSION(2, 6, 29) */
