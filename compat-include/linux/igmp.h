#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_

#include <linux/version.h>
#include_next <linux/igmp.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)

int ip_mc_check_igmp(struct sk_buff *skb, struct sk_buff **skb_trimmed);

#endif /* < KERNEL_VERSION(4, 2, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_IGMP_H_ */
