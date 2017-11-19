/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2017  B.A.T.M.A.N. contributors:
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
 * License-Filename: LICENSES/preferred/GPL-2.0
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)

#define snd_portid snd_pid

#endif /* < KERNEL_VERSION(3, 7, 0) */

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

/* wild hack for batadv_getlink_net only */
#define get_link_net get_xstats_size || 1 ? fallback_net : (struct net*)netdev->rtnl_link_ops->get_xstats_size

#endif /* < KERNEL_VERSION(4, 0, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

#define IFF_NO_QUEUE	0; dev->tx_queue_len = 0

#endif /* < KERNEL_VERSION(4, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)

/* workaround for current issues with Debian's make-kpkg */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
#include <uapi/linux/pkt_cls.h>
#endif

#endif /* < KERNEL_VERSION(4, 6, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)

#define batadv_softif_slave_add(__dev, __slave_dev, __extack) \
	batadv_softif_slave_add(__dev, __slave_dev)

#endif /* < KERNEL_VERSION(4, 15, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
