/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_NL80211_H_
#define _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_NL80211_H_

#include <linux/version.h>
#include_next <uapi/linux/nl80211.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

/* for batadv_v_elp_get_throughput which would have used
 * STATION_INFO_EXPECTED_THROUGHPUT in Linux 4.0.0
 */
#define NL80211_STA_INFO_EXPECTED_THROUGHPUT    28

#endif /* < KERNEL_VERSION(4, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_NL80211_H_ */
