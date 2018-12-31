/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_

#include <linux/version.h>
#include_next <linux/if_bridge.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)

struct br_ip {
	union {
		__be32  ip4;
#if IS_ENABLED(CONFIG_IPV6)
		struct in6_addr ip6;
#endif
	} u;
	__be16          proto;
	__u16           vid;
};

struct br_ip_list {
	struct list_head list;
	struct br_ip addr;
};

#endif /* < KERNEL_VERSION(3, 16, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0) || \
    LINUX_VERSION_CODE == KERNEL_VERSION(3, 16, 0) && \
	(!IS_ENABLED(CONFIG_BRIDGE) || \
	!IS_ENABLED(CONFIG_BRIDGE_IGMP_SNOOPING))

#define br_multicast_list_adjacent(dev, br_ip_list) \
	batadv_br_multicast_list_adjacent(dev, br_ip_list)

#define br_multicast_has_querier_adjacent(dev, proto) \
	batadv_br_multicast_has_querier_adjacent(dev, proto)

static inline int
batadv_br_multicast_list_adjacent(struct net_device *dev,
				  struct list_head *br_ip_list)
{
	return 0;
}

static inline bool
batadv_br_multicast_has_querier_adjacent(struct net_device *dev, int proto)
{
	return false;
}

#endif /* < KERNEL_VERSION(3, 16, 0) ||
	* == KERNEL_VERSION(3, 16, 0) &&
	* (!IS_ENABLED(CONFIG_BRIDGE) ||
	* !IS_ENABLED(CONFIG_BRIDGE_IGMP_SNOOPING)) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)

static inline bool br_multicast_has_querier_anywhere(struct net_device *dev,
						     int proto)
{
	pr_warn_once("Old kernel detected (< 3.17) - multicast optimizations disabled\n");

	return false;
}

#endif /* < KERNEL_VERSION(3, 17, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_ */
