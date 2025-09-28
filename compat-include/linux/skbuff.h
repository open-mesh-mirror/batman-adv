/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
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

#if LINUX_VERSION_IS_LESS(6, 16, 0) || !defined(CONFIG_NET_CRC32C)

#include <linux/crc32.h>

static inline u32 batadv_skb_crc32c(struct sk_buff *skb, int offset,
				    int len, u32 crc)
{
	unsigned int to = offset + len;
	unsigned int consumed = 0;
	struct skb_seq_state st;
	unsigned int l;
	const u8 *data;

	if (len <= 0)
	       return crc;

	skb_prepare_seq_read(skb, offset, to, &st);
	while ((l = skb_seq_read(consumed, &data, &st)) != 0) {
		crc = crc32c(crc, data, l);
		consumed += l;
	}

	return crc;
}

#define skb_crc32c batadv_skb_crc32c

#endif /* LINUX_VERSION_IS_LESS(6, 16, 0) || !defined(CONFIG_NET_CRC32C) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_ */
