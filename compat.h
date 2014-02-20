/* Copyright (C) 2007-2014 B.A.T.M.A.N. contributors:
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#include <linux/version.h>	/* LINUX_VERSION_CODE */

#define consume_skb(_skb) kfree_skb(_skb)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)

#undef __alloc_percpu
#define __alloc_percpu(size, align) \
	percpu_alloc_mask((size), GFP_KERNEL, cpu_possible_map)

#endif /* < KERNEL_VERSION(2, 6, 30) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31)

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p7)

#else

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p6, p7)

#endif /* < KERNEL_VERSION(2, 6, 31) */


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33))
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#include "compat-autoconf.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define __always_unused			__attribute__((unused))
#define __percpu

#define skb_iif iif

#define this_cpu_add(x, c)	batadv_this_cpu_add(&(x), c)

static inline void batadv_this_cpu_add(uint64_t *count_ptr, size_t count)
{
	int cpu = get_cpu();
	*per_cpu_ptr(count_ptr, cpu) += count;
	put_cpu();
}

#define batadv_softif_destroy_netlink(dev, head) batadv_softif_destroy_netlink(dev)
#define unregister_netdevice_queue(dev, head) unregister_netdevice(dev)

static inline struct sk_buff *netdev_alloc_skb_ip_align(struct net_device *dev,
							unsigned int length)
{
	struct sk_buff *skb = netdev_alloc_skb(dev, length + NET_IP_ALIGN);

	if (NET_IP_ALIGN && skb)
		skb_reserve(skb, NET_IP_ALIGN);
	return skb;
}

#define VLAN_PRIO_MASK          0xe000 /* Priority Code Point */
#define VLAN_PRIO_SHIFT         13

#endif /* < KERNEL_VERSION(2, 6, 33) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)

#define rcu_dereference_protected(p, c) (p)

#define rcu_dereference_raw(p)	({ \
				 typeof(p) _________p1 = ACCESS_ONCE(p); \
				 smp_read_barrier_depends(); \
				 (_________p1); \
				 })

#endif /* < KERNEL_VERSION(2, 6, 34) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)

#define pr_warn pr_warning

#endif /* < KERNEL_VERSION(2, 6, 35) */


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

static inline int batadv_param_set_copystring(const char *val,
					      const struct kernel_param *kp)
{
	return param_set_copystring(val, (struct kernel_param *)kp);
}
#define param_set_copystring batadv_param_set_copystring

/* hack for dev->addr_assign_type &= ~NET_ADDR_RANDOM; */
#define addr_assign_type ifindex
#define NET_ADDR_RANDOM 0

#endif /* < KERNEL_VERSION(2, 6, 36) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)

#define hlist_first_rcu(head)	(*((struct hlist_node __rcu **)(&(head)->first)))
#define hlist_next_rcu(node)	(*((struct hlist_node __rcu **)(&(node)->next)))

#endif /* < KERNEL_VERSION(2, 6, 37) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)

#define kstrtou32(cp, base, v)\
({\
	unsigned long _v;\
	int _r;\
	_r = strict_strtoul(cp, base, &_v);\
	*(v) = (uint32_t)_v;\
	if ((unsigned long)*(v) != _v)\
		_r = -ERANGE;\
	_r;\
})
#define kstrtoul strict_strtoul
#define kstrtol  strict_strtol

/* Hack for removing ndo_add/del_slave at the end of net_device_ops.
 * This is somewhat ugly because it requires that ndo_validate_addr
 * is at the end of this struct in soft-interface.c.
 */
#define ndo_validate_addr \
	ndo_validate_addr = eth_validate_addr, \
}; \
static const struct { \
	void *ndo_validate_addr; \
	void *ndo_add_slave; \
	void *ndo_del_slave; \
} __attribute__((unused)) __useless_ops1 = { \
	.ndo_validate_addr

#define ndo_del_slave          ndo_init
#define ndo_init(x, y)         ndo_init - master->netdev_ops->ndo_init - EBUSY

#endif /* < KERNEL_VERSION(2, 6, 39) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)

#define kfree_rcu(ptr, rcu_head) call_rcu(&ptr->rcu_head, batadv_free_rcu_##ptr)
#define vlan_insert_tag(skb, proto, vid) __vlan_put_tag(skb, vid)

void batadv_free_rcu_orig_vlan(struct rcu_head *rcu);
void batadv_free_rcu_softif_vlan(struct rcu_head *rcu);
void batadv_free_rcu_tt_global_entry(struct rcu_head *rcu);
void batadv_free_rcu_gw_node(struct rcu_head *rcu);
void batadv_free_rcu_tt_local_entry(struct rcu_head *rcu);
void batadv_free_rcu_backbone_gw(struct rcu_head *rcu);
void batadv_free_rcu_dat_entry(struct rcu_head *rcu);
void batadv_free_rcu_nc_path(struct rcu_head *rcu);
void batadv_free_rcu_tvlv_handler(struct rcu_head *rcu);

static inline void skb_reset_mac_len(struct sk_buff *skb)
{
	skb->mac_len = skb->network_header - skb->mac_header;
}

#undef BUILD_BUG_ON
#ifdef __CHECKER__
#define BUILD_BUG_ON(condition) (0)
#else /* __CHECKER__ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#endif /* __CHECKER__ */

#endif /* < KERNEL_VERSION(3, 0, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)

#define batadv_interface_add_vid(x, y, z) \
__batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
                          unsigned short vid);\
static void batadv_interface_add_vid(struct net_device *dev, unsigned short vid)\
{\
       __batadv_interface_add_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
                                     unsigned short vid)

#define batadv_interface_kill_vid(x, y, z) \
__batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
                           unsigned short vid);\
static void batadv_interface_kill_vid(struct net_device *dev,\
                                     unsigned short vid)\
{\
       __batadv_interface_kill_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
                                      unsigned short vid)

#endif /* < KERNEL_VERSION(3, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)

#define eth_hw_addr_random(dev)	batadv_eth_hw_addr_random(dev)

static inline void batadv_eth_hw_addr_random(struct net_device *dev)
{
	random_ether_addr(dev->dev_addr);
}

#endif /* < KERNEL_VERSION(3, 4, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)

#ifndef net_ratelimited_function
#define net_ratelimited_function(func, ...) \
	do { \
		if (net_ratelimit()) \
			func(__VA_ARGS__); \
	} while (0)
#endif /* ifndef net_ratelimited_function */

#endif /* < KERNEL_VERSION(3, 5, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)

#define ETH_P_BATMAN	0x4305

/* hack for not correctly set mac_len. This may happen for some special
 * configurations like batman-adv on VLANs.
 *
 * This is pretty dirty, but we only use skb_share_check() in main.c right
 * before mac_len is checked, and the recomputation shouldn't hurt too much.
 */
#define skb_share_check(skb, b) \
	({ \
		struct sk_buff *_t_skb; \
		_t_skb = skb_share_check(skb, b); \
		if (_t_skb) \
			skb_reset_mac_len(_t_skb); \
		_t_skb; \
	})

#endif /* < KERNEL_VERSION(3, 8, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#define prandom_u32() random32()

#define batadv_interface_set_mac_addr(x, y) \
__batadv_interface_set_mac_addr(struct net_device *dev, void *p);\
static int batadv_interface_set_mac_addr(struct net_device *dev, void *p) \
{\
	int ret;\
\
	ret = __batadv_interface_set_mac_addr(dev, p);\
	if (!ret) \
		dev->addr_assign_type &= ~NET_ADDR_RANDOM;\
	return ret;\
}\
static int __batadv_interface_set_mac_addr(x, y)

#define batadv_interface_tx(x, y) \
__batadv_interface_tx(struct sk_buff *skb, struct net_device *soft_iface); \
static int batadv_interface_tx(struct sk_buff *skb, \
			       struct net_device *soft_iface) \
{ \
	skb_reset_mac_header(skb); \
	return __batadv_interface_tx(skb, soft_iface); \
} \
static int __batadv_interface_tx(struct sk_buff *skb, \
				 struct net_device *soft_iface)

#define netdev_master_upper_dev_link netdev_set_master
#define netdev_upper_dev_unlink(slave, master) netdev_set_master(slave, NULL)
#define netdev_master_upper_dev_get(dev) \
({\
	ASSERT_RTNL();\
	dev->master;\
})
#define hlist_entry_safe(ptr, type, member) \
	(ptr) ? hlist_entry(ptr, type, member) : NULL

#undef hlist_for_each_entry
#define hlist_for_each_entry(pos, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);\
	pos; \
	pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#undef hlist_for_each_entry_rcu
#define hlist_for_each_entry_rcu(pos, head, member) \
	for (pos = hlist_entry_safe (rcu_dereference_raw(hlist_first_rcu(head)),\
	typeof(*(pos)), member); \
	pos; \
	pos = hlist_entry_safe(rcu_dereference_raw(hlist_next_rcu(\
	&(pos)->member)), typeof(*(pos)), member))

#undef hlist_for_each_entry_safe
#define hlist_for_each_entry_safe(pos, n, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
	pos && ({ n = pos->member.next; 1; }); \
	pos = hlist_entry_safe(n, typeof(*pos), member))

#endif /* < KERNEL_VERSION(3, 9, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

#ifndef vlan_insert_tag

/* include this header early to let the following define
 * not mess up the original function prototype.
 */
#include <linux/if_vlan.h>
#define vlan_insert_tag(skb, proto, vid) vlan_insert_tag(skb, vid)

#endif /* vlan_insert_tag */

#define NETIF_F_HW_VLAN_CTAG_FILTER NETIF_F_HW_VLAN_FILTER

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)

#define batadv_interface_add_vid(x, y, z) \
__batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
			   unsigned short vid);\
static int batadv_interface_add_vid(struct net_device *dev, unsigned short vid)\
{\
	return __batadv_interface_add_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_add_vid(struct net_device *dev, __be16 proto,\
				      unsigned short vid)

#define batadv_interface_kill_vid(x, y, z) \
__batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
			    unsigned short vid);\
static int batadv_interface_kill_vid(struct net_device *dev,\
				     unsigned short vid)\
{\
	return __batadv_interface_kill_vid(dev, htons(ETH_P_8021Q), vid);\
}\
static int __batadv_interface_kill_vid(struct net_device *dev, __be16 proto,\
				       unsigned short vid)

#endif /* >= KERNEL_VERSION(3, 3, 0) */

#endif /* < KERNEL_VERSION(3, 10, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)

#define netdev_notifier_info_to_dev(ptr) ptr

/* older kernels still need to call skb_abort_seq_read() */
#define skb_seq_read(consumed, data, st) \
	({ \
		int __len = skb_seq_read(consumed, data, st); \
		if (__len == 0) \
			skb_abort_seq_read(st); \
		__len; \
	})
#endif /* < KERNEL_VERSION(3, 11, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0)

#define ether_addr_equal_unaligned(_a, _b) (memcmp(_a, _b, ETH_ALEN) == 0)

#endif /* < KERNEL_VERSION(3, 14, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
