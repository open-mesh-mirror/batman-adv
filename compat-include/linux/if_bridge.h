/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
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

#if LINUX_VERSION_IS_LESS(5, 10, 0)

struct batadv_br_ip {
	union {
		__be32  ip4;
#if IS_ENABLED(CONFIG_IPV6)
		struct in6_addr ip6;
#endif
	} dst;
	__be16          proto;
	__u16           vid;
};

struct batadv_br_ip_list {
	struct list_head list;
	struct batadv_br_ip addr;
};

/* "static" dropped to force compiler to evaluate it as part of multicast.c
 * might need to be added again and then called in some kind of dummy
 * compat.c in case this header is included in multiple files.
 */
inline void __batadv_br_ip_list_check(void)
{
	BUILD_BUG_ON(sizeof(struct batadv_br_ip_list) != sizeof(struct br_ip_list));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip_list, list) != offsetof(struct br_ip_list, list));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip_list, addr) != offsetof(struct br_ip_list, addr));

	BUILD_BUG_ON(sizeof(struct batadv_br_ip) != sizeof(struct br_ip));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip, dst.ip4) != offsetof(struct br_ip, u.ip4));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip, dst.ip6) != offsetof(struct br_ip, u.ip6));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip, proto) != offsetof(struct br_ip, proto));
	BUILD_BUG_ON(offsetof(struct batadv_br_ip, vid) != offsetof(struct br_ip, vid));
}

#define br_ip batadv_br_ip
#define br_ip_list batadv_br_ip_list

#endif /* LINUX_VERSION_IS_LESS(5, 10, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_IF_BRIDGE_H_ */
