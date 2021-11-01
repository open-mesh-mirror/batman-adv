/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_GENETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_GENETLINK_H_

#include <linux/version.h>
#include_next <uapi/linux/genetlink.h>

#if LINUX_VERSION_IS_LESS(4, 6, 0)

#define GENL_UNS_ADMIN_PERM GENL_ADMIN_PERM

#endif /* LINUX_VERSION_IS_LESS(4, 6, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_GENETLINK_H_ */
