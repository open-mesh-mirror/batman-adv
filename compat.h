/* Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
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
#include <linux/kconfig.h>
#include <generated/autoconf.h>

#include "compat-autoconf.h"

/* test for dependency CONFIG_BATMAN_ADV_DEBUG -> CONFIG_BATMAN_ADV_DEBUGFS */
#if defined(CONFIG_BATMAN_ADV_DEBUG) && !defined(CONFIG_BATMAN_ADV_DEBUGFS)
#error CONFIG_BATMAN_ADV_DEBUG=y requires CONFIG_BATMAN_ADV_DEBUGFS=y
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#include <linux/netdevice.h>

#define netdev_master_upper_dev_get_rcu(dev) \
	(dev->priv_flags & IFF_BRIDGE_PORT ? dev : NULL); \
	break;

#endif /* < KERNEL_VERSION(3, 9, 0) */

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

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

#endif /* < KERNEL_VERSION(3, 9, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0)

/* the expected behaviour of this function is to return 0 on success, therefore
 * it is possible to define it as 1 so that batman-adv thinks like something
 * went wrong. It will then decide what to do.
 */
#define cfg80211_get_station(_a, _b, _c) (1)
/* the following define substitute the expected_throughput field with a random
 * one existing in the station_info struct. It can be random because due to the
 * define above it will never be used. We need it only to make the code compile
 */
#define expected_throughput filled

#ifdef CONFIG_BATMAN_ADV_BATMAN_V

#warning cfg80211 based throughput metric is only supported with Linux 3.15+

#endif

#endif /* < KERNEL_VERSION(3, 15, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

/* WARNING for batadv_getlink_net */
#define get_link_net get_xstats_size || 1 || netdev->rtnl_link_ops->get_xstats_size

#endif /* < KERNEL_VERSION(4, 0, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

#define IFF_NO_QUEUE	0; dev->tx_queue_len = 0

#endif /* < KERNEL_VERSION(4, 3, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
