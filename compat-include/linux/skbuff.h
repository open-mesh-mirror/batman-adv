/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_

#include <linux/version.h>
#include_next <linux/skbuff.h>

#if LINUX_VERSION_IS_LESS(4, 5, 0)

static inline void batadv_skb_postpush_rcsum(struct sk_buff *skb,
					     const void *start,
					     unsigned int len)
{
	if (skb->ip_summed == CHECKSUM_COMPLETE)
		skb->csum = csum_block_add(skb->csum,
					   csum_partial(start, len, 0), 0);
}

#define skb_postpush_rcsum batadv_skb_postpush_rcsum

#endif /* LINUX_VERSION_IS_LESS(4, 5, 0) */

#if LINUX_VERSION_IS_LESS(4, 13, 0)

static inline void *batadv_skb_put(struct sk_buff *skb, unsigned int len)
{
	return (void *)skb_put(skb, len);
}
#define skb_put batadv_skb_put

static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memset(tmp, 0, len);

	return tmp;
}

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
				 unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memcpy(tmp, data, len);

	return tmp;
}

#endif /* LINUX_VERSION_IS_LESS(4, 13, 0) */

#if LINUX_VERSION_IS_LESS(5, 4, 0)

#define nf_reset_ct nf_reset

#endif /* LINUX_VERSION_IS_LESS(5, 4, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_ */
