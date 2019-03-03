/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_NETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_NET_NETLINK_H_

#include <linux/version.h>
#include_next <net/netlink.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)

static inline int nla_put_in_addr(struct sk_buff *skb, int attrtype,
				  __be32 addr)
{
	__be32 tmp = addr;

	return nla_put_be32(skb, attrtype, tmp);
}

#endif /* < KERNEL_VERSION(4, 1, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_NETLINK_H_ */
