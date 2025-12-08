/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_

#include <linux/version.h>
#include_next <linux/if_bridge.h>

#if LINUX_VERSION_IS_LESS(5, 14, 0)

#include <net/addrconf.h>

#if IS_ENABLED(CONFIG_IPV6)
static inline bool
br_multicast_has_router_adjacent(struct net_device *dev, int proto)
{
	struct list_head bridge_mcast_list = LIST_HEAD_INIT(bridge_mcast_list);
	struct br_ip_list *br_ip_entry, *tmp;
	int ret;

	if (proto != ETH_P_IPV6)
		return true;

	ret = br_multicast_list_adjacent(dev, &bridge_mcast_list);
	if (ret < 0)
		return true;

	ret = false;

	list_for_each_entry_safe(br_ip_entry, tmp, &bridge_mcast_list, list) {
		if (br_ip_entry->addr.proto == htons(ETH_P_IPV6) &&
		    ipv6_addr_is_ll_all_routers(&br_ip_entry->addr.dst.ip6))
			ret = true;

		list_del(&br_ip_entry->list);
		kfree(br_ip_entry);
	}

	return ret;
}
#else
static inline bool
br_multicast_has_router_adjacent(struct net_device *dev, int proto)
{
	return true;
}
#endif

#endif /* LINUX_VERSION_IS_LESS(5, 14, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_ */
