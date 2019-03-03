/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2016-2019  B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_ETHTOOL_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_ETHTOOL_H_

#include <linux/version.h>
#include_next <linux/ethtool.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)

#define ethtool_link_ksettings batadv_ethtool_link_ksettings

struct batadv_ethtool_link_ksettings {
	struct {
		__u32	speed;
		__u8	duplex;
		__u8	autoneg;
	} base;
};

#define __ethtool_get_link_ksettings(__dev, __link_settings) \
	batadv_ethtool_get_link_ksettings(__dev, __link_settings)

static inline int
batadv_ethtool_get_link_ksettings(struct net_device *dev,
				  struct ethtool_link_ksettings *link_ksettings)
{
	struct ethtool_cmd cmd;
	int ret;

	memset(&cmd, 0, sizeof(cmd));
	ret = __ethtool_get_settings(dev, &cmd);

	if (ret != 0)
		return ret;

	link_ksettings->base.duplex = cmd.duplex;
	link_ksettings->base.autoneg = cmd.autoneg;
	link_ksettings->base.speed = ethtool_cmd_speed(&cmd);

	return 0;
}

#endif /* < KERNEL_VERSION(4, 6, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_ETHTOOL_H_ */
