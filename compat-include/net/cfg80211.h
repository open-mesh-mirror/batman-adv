/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_
#define _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_

#include <linux/version.h>
#include_next <net/cfg80211.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)

static inline int cfg80211_get_station(struct net_device *dev,
				       const u8 *mac_addr,
				       struct station_info *sinfo)
{
	pr_warn_once("cfg80211 based throughput metric is only supported with Linux 3.16+\n");
	return -ENOENT;
}

/* The following define substitutes the expected_throughput field with a random
 * one existing in the station_info struct. It can be random because due to the
 * function above it will never be used. Only needed to make the code compile
 */
#define expected_throughput filled

#endif /* < KERNEL_VERSION(3, 16, 0) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)

#if !IS_ENABLED(CONFIG_CFG80211) && \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)

#define cfg80211_get_station(dev, mac_addr, sinfo) \
	batadv_cfg80211_get_station(dev, mac_addr, sinfo)

static inline int batadv_cfg80211_get_station(struct net_device *dev,
					      const u8 *mac_addr,
					      struct station_info *sinfo)
{
	return -ENOENT;
}
#endif

#endif /* < KERNEL_VERSION(4, 8, 0) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0) && IS_ENABLED(CONFIG_CFG80211)

/* cfg80211 fix: https://patchwork.kernel.org/patch/10449857/ */
static inline int batadv_cfg80211_get_station(struct net_device *dev,
					      const u8 *mac_addr,
					      struct station_info *sinfo)
{
	memset(sinfo, 0, sizeof(*sinfo));
	return cfg80211_get_station(dev, mac_addr, sinfo);
}

#define cfg80211_get_station(dev, mac_addr, sinfo) \
	batadv_cfg80211_get_station(dev, mac_addr, sinfo)

#endif /* < KERNEL_VERSION(4, 18, 0) && IS_ENABLED(CONFIG_CFG80211) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)

#define cfg80211_sinfo_release_content(sinfo)

#endif /* < KERNEL_VERSION(4, 18, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_ */
