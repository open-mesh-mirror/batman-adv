/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 */

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#ifdef __KERNEL__

#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include <linux/kconfig.h>
#include <generated/autoconf.h>

#include "compat-autoconf.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

/* wild hack for batadv_getlink_net only */
#define get_link_net get_xstats_size || 1 ? fallback_net : (struct net*)netdev->rtnl_link_ops->get_xstats_size

#endif /* < KERNEL_VERSION(4, 0, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

#define IFF_NO_QUEUE	0; dev->tx_queue_len = 0

#endif /* < KERNEL_VERSION(4, 3, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)

/* workaround for current issues with Debian's make-kpkg */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
#include <uapi/linux/pkt_cls.h>
#endif

#endif /* < KERNEL_VERSION(4, 6, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)

#define batadv_softif_slave_add(__dev, __slave_dev, __extack) \
	batadv_softif_slave_add(__dev, __slave_dev)

#endif /* < KERNEL_VERSION(4, 15, 0) */

#endif /* __KERNEL__ */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
