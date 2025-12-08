/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_

#include <linux/version.h>
#include_next <net/genetlink.h>

#if LINUX_VERSION_IS_LESS(6, 2, 0)

#define genl_split_ops genl_ops

#endif /* LINUX_VERSION_IS_LESS(6, 2, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_ */
