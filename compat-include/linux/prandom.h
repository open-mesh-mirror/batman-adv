/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_PRANDOM_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_PRANDOM_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(5, 8, 1) || \
    (LINUX_VERSION_IS_GEQ(4, 4, 233) && LINUX_VERSION_IS_LESS(4, 5, 0)) || \
    (LINUX_VERSION_IS_GEQ(4, 9, 233) && LINUX_VERSION_IS_LESS(4, 10, 0)) || \
    (LINUX_VERSION_IS_GEQ(4, 14, 193) && LINUX_VERSION_IS_LESS(4, 15, 0)) || \
    (LINUX_VERSION_IS_GEQ(4, 19, 138) && LINUX_VERSION_IS_LESS(4, 20, 0)) || \
    (LINUX_VERSION_IS_GEQ(5, 4, 57) && LINUX_VERSION_IS_LESS(5, 5, 0)) || \
    (LINUX_VERSION_IS_GEQ(5, 7, 14) && LINUX_VERSION_IS_LESS(5, 8, 0))
#include_next <linux/prandom.h>
#else
#include <linux/random.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_PRANDOM_H_ */
