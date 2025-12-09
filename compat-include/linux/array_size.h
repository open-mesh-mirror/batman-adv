/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_ARRAY_SIZE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_ARRAY_SIZE_H_

#include <linux/version.h>
#if (LINUX_VERSION_IS_GEQ(5, 15, 197) && LINUX_VERSION_IS_LESS(5, 16, 0)) || \
    (LINUX_VERSION_IS_GEQ(6, 1, 159) && LINUX_VERSION_IS_LESS(6, 2, 0)) || \
    (LINUX_VERSION_IS_GEQ(6, 6, 118) && LINUX_VERSION_IS_LESS(6, 7, 0)) || \
    LINUX_VERSION_IS_GEQ(6, 7, 0)
#include_next <linux/array_size.h>
#else
#include <linux/kernel.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_ARRAY_SIZE_H_ */
