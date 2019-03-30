/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H
#define _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H

#include <linux/version.h>
#include_next <linux/cache.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)

#define __ro_after_init

#endif /* < KERNEL_VERSION(4, 6, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H */
