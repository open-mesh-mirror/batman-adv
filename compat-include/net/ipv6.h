#ifndef _NET_BATMAN_ADV_COMPAT_NET_IPV6_H_
#define _NET_BATMAN_ADV_COMPAT_NET_IPV6_H_

#include <linux/version.h>
#include_next <net/ipv6.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)

#define ipv6_skip_exthdr(skb, start, nexthdrp, frag_offp) \
	({ \
		(void)frag_offp; \
		ipv6_skip_exthdr(skb, start, nexthdrp); \
	})

#endif /* < KERNEL_VERSION(3, 3, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_IPV6_H_ */
