#ifndef _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_
#define _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_

#include <linux/version.h>
#include_next <net/addrconf.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)

int ipv6_mc_check_mld(struct sk_buff *skb, struct sk_buff **skb_trimmed);

#endif /* < KERNEL_VERSION(4, 2, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_ */
