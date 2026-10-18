/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_

#include <linux/version.h>
#include_next <linux/netdevice.h>

#if LINUX_VERSION_IS_LESS(6, 0, 0)

#define netdev_hold(__dev, __tracker, __gfp) \
	dev_hold_track(__dev, __tracker, __gfp)

#define netdev_put(__dev, __tracker) \
	dev_put_track(__dev, __tracker)

#endif /* LINUX_VERSION_IS_LESS(6, 0, 0) */

#if LINUX_VERSION_IS_LESS(6, 15, 0) && \
    !(LINUX_VERSION_IS_GEQ(6, 12, 114) && LINUX_VERSION_IS_LESS(6, 13, 0))

#define netns_immutable netns_local

#endif /* LINUX_VERSION_IS_LESS(6, 15, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_ */
