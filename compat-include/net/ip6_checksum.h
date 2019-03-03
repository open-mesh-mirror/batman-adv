/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_IP6_CHECKSUM_H_
#define _NET_BATMAN_ADV_COMPAT_NET_IP6_CHECKSUM_H_

#include <linux/version.h>
#include_next <net/ip6_checksum.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)

static inline __wsum ip6_compute_pseudo(struct sk_buff *skb, int proto)
{
	return ~csum_unfold(csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
					    &ipv6_hdr(skb)->daddr,
					    skb->len, proto, 0));
}

#endif /* < KERNEL_VERSION(3, 16, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_IP6_CHECKSUM_H_ */
