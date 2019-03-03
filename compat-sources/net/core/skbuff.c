// SPDX-License-Identifier: GPL-2.0
/*
 *	Routines having to do with the 'struct sk_buff' memory handlers.
 *
 *	Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 *			Florian La Roche <rzsfl@rz.uni-sb.de>
 *
 *	Fixes:
 *		Alan Cox	:	Fixed the worst of the load
 *					balancer bugs.
 *		Dave Platt	:	Interrupt stacking fix.
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	Changed buffer format.
 *		Alan Cox	:	destructor hook for AF_UNIX etc.
 *		Linus Torvalds	:	Better skb_clone.
 *		Alan Cox	:	Added skb_copy.
 *		Alan Cox	:	Added all the changed routines Linus
 *					only put in the headers
 *		Ray VanTassle	:	Fixed --skb->lock in free
 *		Alan Cox	:	skb_copy copy arp field
 *		Andi Kleen	:	slabified it.
 *		Robert Olsson	:	Removed skb_head_pool
 *
 *	NOTE:
 *		The __skb_ routines should be called with interrupts
 *	disabled, or you better be *real* sure that the operation is atomic
 *	with respect to whatever list is being frobbed (e.g. via lock_sock()
 *	or via disabling bottom half handlers, etc).
 */

#include <linux/in6.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)

/* Compare with:
 * "bridge: multicast: call skb_checksum_{simple_, }validate"
 */
__sum16 skb_checksum_simple_validate(struct sk_buff *skb)
{
	switch (skb->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (!csum_fold(skb->csum))
			break;
		/* fall through */
	case CHECKSUM_NONE:
		skb->csum = 0;
		return skb_checksum_complete(skb);
	}

	return 0;
}

/* Watch out: Not as generic as upstream
 * - redefines this method to only fit with ICMPV6
 *
 * Compare with:
 * "bridge: multicast: call skb_checksum_{simple_, }validate"
 */
__sum16
skb_checksum_validate(struct sk_buff *skb, int proto,
		      __wsum (*compute_pseudo)(struct sk_buff *skb, int proto))
{
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);

	switch (skb->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (!csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr, skb->len,
				     IPPROTO_ICMPV6, skb->csum))
			break;
		/*FALLTHROUGH*/
	case CHECKSUM_NONE:
		skb->csum = ~csum_unfold(csum_ipv6_magic(&ip6h->saddr,
							 &ip6h->daddr,
							 skb->len,
							 IPPROTO_ICMPV6, 0));
		return __skb_checksum_complete(skb);
	}

	return 0;
}

#endif /* < KERNEL_VERSION(3, 16, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)

/**
 *	skb_push_rcsum - push skb and update receive checksum
 *	@skb: buffer to update
 *	@len: length of data pulled
 *
 *	This function performs an skb_push on the packet and updates
 *	the CHECKSUM_COMPLETE checksum.  It should be used on
 *	receive path processing instead of skb_push unless you know
 *	that the checksum difference is zero (e.g., a valid IP header)
 *	or you are setting ip_summed to CHECKSUM_NONE.
 */
static unsigned char *skb_push_rcsum(struct sk_buff *skb, unsigned len)
{
	skb_push(skb, len);
	skb_postpush_rcsum(skb, skb->data, len);
	return skb->data;
}

/**
 * skb_checksum_maybe_trim - maybe trims the given skb
 * @skb: the skb to check
 * @transport_len: the data length beyond the network header
 *
 * Checks whether the given skb has data beyond the given transport length.
 * If so, returns a cloned skb trimmed to this transport length.
 * Otherwise returns the provided skb. Returns NULL in error cases
 * (e.g. transport_len exceeds skb length or out-of-memory).
 *
 * Caller needs to set the skb transport header and release the returned skb.
 * Provided skb is consumed.
 */
static struct sk_buff *skb_checksum_maybe_trim(struct sk_buff *skb,
					       unsigned int transport_len)
{
	struct sk_buff *skb_chk;
	unsigned int len = skb_transport_offset(skb) + transport_len;
	int ret;

	if (skb->len < len) {
		kfree_skb(skb);
		return NULL;
	} else if (skb->len == len) {
		return skb;
	}

	skb_chk = skb_clone(skb, GFP_ATOMIC);
	kfree_skb(skb);

	if (!skb_chk)
		return NULL;

	ret = pskb_trim_rcsum(skb_chk, len);
	if (ret) {
		kfree_skb(skb_chk);
		return NULL;
	}

	return skb_chk;
}

/**
 * skb_checksum_trimmed - validate checksum of an skb
 * @skb: the skb to check
 * @transport_len: the data length beyond the network header
 * @skb_chkf: checksum function to use
 *
 * Applies the given checksum function skb_chkf to the provided skb.
 * Returns a checked and maybe trimmed skb. Returns NULL on error.
 *
 * If the skb has data beyond the given transport length, then a
 * trimmed & cloned skb is checked and returned.
 *
 * Caller needs to set the skb transport header and release the returned skb.
 * Provided skb is consumed.
 */
struct sk_buff *skb_checksum_trimmed(struct sk_buff *skb,
				     unsigned int transport_len,
				     __sum16(*skb_chkf)(struct sk_buff *skb))
{
	struct sk_buff *skb_chk;
	unsigned int offset = skb_transport_offset(skb);
	__sum16 ret;

	skb_chk = skb_checksum_maybe_trim(skb, transport_len);
	if (!skb_chk)
		return NULL;

	if (!pskb_may_pull(skb_chk, offset)) {
		kfree_skb(skb_chk);
		return NULL;
	}

	skb_pull_rcsum(skb_chk, offset);
	ret = skb_chkf(skb_chk);
	skb_push_rcsum(skb_chk, offset);

	if (ret) {
		kfree_skb(skb_chk);
		return NULL;
	}

	return skb_chk;
}

#endif /* < KERNEL_VERSION(4, 2, 0) */
