// SPDX-License-Identifier: GPL-2.0
/*
 *	Linux NET3:	Internet Group Management Protocol  [IGMP]
 *
 *	This code implements the IGMP protocol as defined in RFC1112. There has
 *	been a further revision of this protocol since which is now supported.
 *
 *	If you have trouble with this module be careful what gcc you have used,
 *	the older version didn't come out right using gcc 2.5.8, the newer one
 *	seems to fall out with gcc 2.6.2.
 *
 *	Authors:
 *		Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Fixes:
 *
 *		Alan Cox	:	Added lots of __inline__ to optimise
 *					the memory usage of all the tiny little
 *					functions.
 *		Alan Cox	:	Dumped the header building experiment.
 *		Alan Cox	:	Minor tweaks ready for multicast routing
 *					and extended IGMP protocol.
 *		Alan Cox	:	Removed a load of inline directives. Gcc 2.5.8
 *					writes utterly bogus code otherwise (sigh)
 *					fixed IGMP loopback to behave in the manner
 *					desired by mrouted, fixed the fact it has been
 *					broken since 1.3.6 and cleaned up a few minor
 *					points.
 *
 *		Chih-Jen Chang	:	Tried to revise IGMP to Version 2
 *		Tsu-Sheng Tsao		E-mail: chihjenc@scf.usc.edu and tsusheng@scf.usc.edu
 *					The enhancements are mainly based on Steve Deering's
 * 					ipmulti-3.5 source code.
 *		Chih-Jen Chang	:	Added the igmp_get_mrouter_info and
 *		Tsu-Sheng Tsao		igmp_set_mrouter_info to keep track of
 *					the mrouted version on that device.
 *		Chih-Jen Chang	:	Added the max_resp_time parameter to
 *		Tsu-Sheng Tsao		igmp_heard_query(). Using this parameter
 *					to identify the multicast router version
 *					and do what the IGMP version 2 specified.
 *		Chih-Jen Chang	:	Added a timer to revert to IGMP V2 router
 *		Tsu-Sheng Tsao		if the specified time expired.
 *		Alan Cox	:	Stop IGMP from 0.0.0.0 being accepted.
 *		Alan Cox	:	Use GFP_ATOMIC in the right places.
 *		Christian Daudt :	igmp timer wasn't set for local group
 *					memberships but was being deleted,
 *					which caused a "del_timer() called
 *					from %p with timer not initialized\n"
 *					message (960131).
 *		Christian Daudt :	removed del_timer from
 *					igmp_timer_expire function (960205).
 *             Christian Daudt :       igmp_heard_report now only calls
 *                                     igmp_timer_expire if tm->running is
 *                                     true (960216).
 *		Malcolm Beattie :	ttl comparison wrong in igmp_rcv made
 *					igmp_heard_query never trigger. Expiry
 *					miscalculation fixed in igmp_heard_query
 *					and random() made to return unsigned to
 *					prevent negative expiry times.
 *		Alexey Kuznetsov:	Wrong group leaving behaviour, backport
 *					fix from pending 2.1.x patches.
 *		Alan Cox:		Forget to enable FDDI support earlier.
 *		Alexey Kuznetsov:	Fixed leaving groups on device down.
 *		Alexey Kuznetsov:	Accordance to igmp-v2-06 draft.
 *		David L Stevens:	IGMPv3 support, with help from
 *					Vinay Kulkarni
 */

#include <linux/igmp.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <net/ip.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)

static int ip_mc_check_iphdr(struct sk_buff *skb)
{
	const struct iphdr *iph;
	unsigned int len;
	unsigned int offset = skb_network_offset(skb) + sizeof(*iph);

	if (!pskb_may_pull(skb, offset))
		return -EINVAL;

	iph = ip_hdr(skb);

	if (iph->version != 4 || ip_hdrlen(skb) < sizeof(*iph))
		return -EINVAL;

	offset += ip_hdrlen(skb) - sizeof(*iph);

	if (!pskb_may_pull(skb, offset))
		return -EINVAL;

	iph = ip_hdr(skb);

	if (unlikely(ip_fast_csum((u8 *)iph, iph->ihl)))
		return -EINVAL;

	len = skb_network_offset(skb) + ntohs(iph->tot_len);
	if (skb->len < len || len < offset)
		return -EINVAL;

	skb_set_transport_header(skb, offset);

	return 0;
}

static int ip_mc_check_igmp_reportv3(struct sk_buff *skb)
{
	unsigned int len = skb_transport_offset(skb);

	len += sizeof(struct igmpv3_report);

	return pskb_may_pull(skb, len) ? 0 : -EINVAL;
}

static int ip_mc_check_igmp_query(struct sk_buff *skb)
{
	unsigned int len = skb_transport_offset(skb);

	len += sizeof(struct igmphdr);
	if (skb->len < len)
		return -EINVAL;

	/* IGMPv{1,2}? */
	if (skb->len != len) {
		/* or IGMPv3? */
		len += sizeof(struct igmpv3_query) - sizeof(struct igmphdr);
		if (skb->len < len || !pskb_may_pull(skb, len))
			return -EINVAL;
	}

	/* RFC2236+RFC3376 (IGMPv2+IGMPv3) require the multicast link layer
	 * all-systems destination addresses (224.0.0.1) for general queries
	 */
	if (!igmp_hdr(skb)->group &&
	    ip_hdr(skb)->daddr != htonl(INADDR_ALLHOSTS_GROUP))
		return -EINVAL;

	return 0;
}

static int ip_mc_check_igmp_msg(struct sk_buff *skb)
{
	switch (igmp_hdr(skb)->type) {
	case IGMP_HOST_LEAVE_MESSAGE:
	case IGMP_HOST_MEMBERSHIP_REPORT:
	case IGMPV2_HOST_MEMBERSHIP_REPORT:
		/* fall through */
		return 0;
	case IGMPV3_HOST_MEMBERSHIP_REPORT:
		return ip_mc_check_igmp_reportv3(skb);
	case IGMP_HOST_MEMBERSHIP_QUERY:
		return ip_mc_check_igmp_query(skb);
	default:
		return -ENOMSG;
	}
}

static inline __sum16 ip_mc_validate_checksum(struct sk_buff *skb)
{
	return skb_checksum_simple_validate(skb);
}

static int __ip_mc_check_igmp(struct sk_buff *skb)

{
	struct sk_buff *skb_chk;
	unsigned int transport_len;
	unsigned int len = skb_transport_offset(skb) + sizeof(struct igmphdr);
	int ret;

	transport_len = ntohs(ip_hdr(skb)->tot_len) - ip_hdrlen(skb);

	skb_get(skb);
	skb_chk = skb_checksum_trimmed(skb, transport_len,
				       ip_mc_validate_checksum);
	if (!skb_chk)
		return -EINVAL;

	if (!pskb_may_pull(skb_chk, len)) {
		kfree_skb(skb_chk);
		return -EINVAL;
	}

	ret = ip_mc_check_igmp_msg(skb_chk);
	if (ret) {
		kfree_skb(skb_chk);
		return ret;
	}

	kfree_skb(skb_chk);

	return 0;
}

/**
 * ip_mc_check_igmp - checks whether this is a sane IGMP packet
 * @skb: the skb to validate
 *
 * Checks whether an IPv4 packet is a valid IGMP packet. If so sets
 * skb network and transport headers accordingly and returns zero.
 *
 * -EINVAL: A broken packet was detected, i.e. it violates some internet
 *  standard
 * -ENOMSG: IP header validation succeeded but it is not an IGMP packet.
 * -ENOMEM: A memory allocation failure happened.
 */
int ip_mc_check_igmp(struct sk_buff *skb)
{
	int ret = ip_mc_check_iphdr(skb);

	if (ret < 0)
		return ret;

	if (ip_hdr(skb)->protocol != IPPROTO_IGMP)
		return -ENOMSG;

	return __ip_mc_check_igmp(skb);
}

#endif /* < KERNEL_VERSION(4, 2, 0) */
