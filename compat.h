/*
 * Copyright (C) 2007-2012 B.A.T.M.A.N. contributors:
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

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#include <linux/version.h>	/* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33))
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#include "compat-autoconf.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define __always_unused			__attribute__((unused))

#define skb_iif iif

#endif /* < KERNEL_VERSION(2, 6, 33) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)

#define hlist_first_rcu(head) (*((struct hlist_node **)(&(head)->first)))
#define hlist_next_rcu(node) (*((struct hlist_node **)(&(node)->next)))

#define __hlist_for_each_rcu(pos, head) \
	for (pos = rcu_dereference(hlist_first_rcu(head)); \
	pos && ({ prefetch(pos->next); 1; }); \
	pos = rcu_dereference(hlist_next_rcu(pos)))

#define rcu_dereference_protected(p, c) (p)

#endif /* < KERNEL_VERSION(2, 6, 34) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31)

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p7)

#else

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p6, p7)

#endif /* < KERNEL_VERSION(2, 6, 31) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

#define __rcu
#define IFF_BRIDGE_PORT  0 || (hard_iface->net_dev->br_port ? 1 : 0)

struct kernel_param_ops {
	/* Returns 0, or -errno.  arg is in kp->arg. */
	int (*set)(const char *val, const struct kernel_param *kp);
	/* Returns length written or -errno.  Buffer is 4k (ie. be short!) */
	int (*get)(char *buffer, struct kernel_param *kp);
	/* Optional function to free kp->arg when module unloaded. */
	void (*free)(void *arg);
};

#define module_param_cb(name, ops, arg, perm)				\
	static int __compat_set_param_##name(const char *val,		\
					     struct kernel_param *kp)	\
				{ return (ops)->set(val, kp); }		\
	static int __compat_get_param_##name(char *buffer,		\
					     struct kernel_param *kp)	\
				{ return (ops)->get(buffer, kp); }	\
	__compat__module_param_call(MODULE_PARAM_PREFIX, name,		\
				    __compat_set_param_##name,		\
				    __compat_get_param_##name, arg,	\
				    __same_type((arg), bool *), perm)

static inline int __param_set_copystring(const char *val,
					 const struct kernel_param *kp)
{
	return param_set_copystring(val, (struct kernel_param *)kp);
}
#define param_set_copystring __param_set_copystring

#endif /* < KERNEL_VERSION(2, 6, 36) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)

#define kstrtoul strict_strtoul
#define kstrtol  strict_strtol

#endif /* < KERNEL_VERSION(2, 6, 39) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)

#define kfree_rcu(ptr, rcu_head) call_rcu(&ptr->rcu_head, free_rcu_##ptr)
#define vlan_insert_tag(skb, vid) __vlan_put_tag(skb, vid)

void free_rcu_gw_node(struct rcu_head *rcu);
void free_rcu_neigh_node(struct rcu_head *rcu);
void free_rcu_tt_local_entry(struct rcu_head *rcu);
void free_rcu_backbone_gw(struct rcu_head *rcu);

#endif /* < KERNEL_VERSION(3, 0, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
