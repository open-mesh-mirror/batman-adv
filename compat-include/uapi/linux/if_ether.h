/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_IF_ETHER_H_
#define _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_IF_ETHER_H_

#include <linux/version.h>
#include_next <uapi/linux/if_ether.h>


#if LINUX_VERSION_IS_LESS(4, 10, 0)

#ifndef ETH_MIN_MTU
#define ETH_MIN_MTU	68
#endif

#endif /* LINUX_VERSION_IS_LESS(4, 10, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_IF_ETHER_H_ */
