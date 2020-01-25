/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
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

#if LINUX_VERSION_IS_LESS(4, 6, 0)

#define __ro_after_init

#endif /* LINUX_VERSION_IS_LESS(4, 6, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CACHE_H */
