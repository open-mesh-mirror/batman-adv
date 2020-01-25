/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETLINK_H_

#include <linux/version.h>
#include_next <linux/netlink.h>

#if LINUX_VERSION_IS_LESS(4, 7, 0)

#include_next <net/netlink.h>

static inline bool batadv_nla_need_padding_for_64bit(struct sk_buff *skb);

static inline int batadv_nla_align_64bit(struct sk_buff *skb, int padattr)
{
	if (batadv_nla_need_padding_for_64bit(skb) &&
	    !nla_reserve(skb, padattr, 0))
		return -EMSGSIZE;

	return 0;
}

static inline struct nlattr *batadv__nla_reserve_64bit(struct sk_buff *skb,
						       int attrtype,
						       int attrlen, int padattr)
{
	if (batadv_nla_need_padding_for_64bit(skb))
		batadv_nla_align_64bit(skb, padattr);

	return __nla_reserve(skb, attrtype, attrlen);
}

static inline void batadv__nla_put_64bit(struct sk_buff *skb, int attrtype,
					 int attrlen, const void *data,
					 int padattr)
{
	struct nlattr *nla;

	nla = batadv__nla_reserve_64bit(skb, attrtype, attrlen, padattr);
	memcpy(nla_data(nla), data, attrlen);
}

static inline bool batadv_nla_need_padding_for_64bit(struct sk_buff *skb)
{
#ifndef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
	/* The nlattr header is 4 bytes in size, that's why we test
	 * if the skb->data _is_ aligned.  A NOP attribute, plus
	 * nlattr header for next attribute, will make nla_data()
	 * 8-byte aligned.
	 */
	if (IS_ALIGNED((unsigned long)skb_tail_pointer(skb), 8))
		return true;
#endif
	return false;
}

static inline int batadv_nla_total_size_64bit(int payload)
{
	return NLA_ALIGN(nla_attr_size(payload))
#ifndef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
		+ NLA_ALIGN(nla_attr_size(0))
#endif
		;
}

static inline int batadv_nla_put_64bit(struct sk_buff *skb, int attrtype,
				       int attrlen, const void *data,
				       int padattr)
{
	size_t len;

	if (batadv_nla_need_padding_for_64bit(skb))
		len = batadv_nla_total_size_64bit(attrlen);
	else
		len = nla_total_size(attrlen);
	if (unlikely(skb_tailroom(skb) < len))
		return -EMSGSIZE;

	batadv__nla_put_64bit(skb, attrtype, attrlen, data, padattr);
	return 0;
}

#define nla_put_u64_64bit(_skb, _attrtype, _value, _padattr) \
	batadv_nla_put_u64_64bit(_skb, _attrtype, _value, _padattr)
static inline int batadv_nla_put_u64_64bit(struct sk_buff *skb, int attrtype,
					   u64 value, int padattr)
{
	return batadv_nla_put_64bit(skb, attrtype, sizeof(u64), &value,
				    padattr);
}

#endif /* LINUX_VERSION_IS_LESS(4, 7, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETLINK_H_ */
