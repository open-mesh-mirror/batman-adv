/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_

#include <linux/version.h>
#include_next <linux/netdevice.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)

/* alloc_netdev() was defined differently before 2.6.38 */
#undef alloc_netdev
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mqs(sizeof_priv, name, setup, 1, 1)

#endif /* < KERNEL_VERSION(3, 17, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)

#define dev_get_iflink(_net_dev) ((_net_dev)->iflink)

#endif /* < KERNEL_VERSION(3, 19, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info, extack) \
	netdev_master_upper_dev_link(dev, upper_dev)

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)

#define netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info, extack) \
	netdev_master_upper_dev_link(dev, upper_dev, upper_priv, upper_info)

#endif /* < KERNEL_VERSION(4, 15, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0)

#define netif_trans_update batadv_netif_trans_update
static inline void batadv_netif_trans_update(struct net_device *dev)
{
	dev->trans_start = jiffies;
}

#endif /* < KERNEL_VERSION(4, 7, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 9)

/* work around missing attribute needs_free_netdev and priv_destructor in
 * net_device
 */
#define ether_setup(dev) \
	void batadv_softif_free2(struct net_device *dev) \
	{ \
		batadv_softif_free(dev); \
		free_netdev(dev); \
	} \
	void (*t1)(struct net_device *dev) __attribute__((unused)); \
	bool t2 __attribute__((unused)); \
	ether_setup(dev)
#define needs_free_netdev destructor = batadv_softif_free2; t2
#define priv_destructor destructor = batadv_softif_free2; t1

#endif /* < KERNEL_VERSION(4, 11, 9) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NETDEVICE_H_ */
