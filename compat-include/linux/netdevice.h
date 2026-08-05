/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_

#include <linux/version.h>
#include_next <linux/netdevice.h>

#if LINUX_VERSION_IS_LESS(5, 17, 0)

typedef struct {} netdevice_tracker;

#define netdev_hold(__dev, __tracker, __gfp) \
	dev_hold(__dev)

#define netdev_put(__dev, __tracker) \
	dev_put(__dev)

#elif LINUX_VERSION_IS_LESS(6, 0, 0)

#define netdev_hold(__dev, __tracker, __gfp) \
	dev_hold_track(__dev, __tracker, __gfp)

#define netdev_put(__dev, __tracker) \
	dev_put_track(__dev, __tracker)

#endif /* LINUX_VERSION_IS_LESS(5, 17, 0) */

#if LINUX_VERSION_IS_LESS(5, 18, 0)

static inline int batadv_netif_rx(struct sk_buff *skb)
{
	if (in_interrupt())
		return netif_rx(skb);
	else
		return netif_rx_ni(skb);
}
#define netif_rx batadv_netif_rx

#endif /* LINUX_VERSION_IS_LESS(5, 18, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_ */
