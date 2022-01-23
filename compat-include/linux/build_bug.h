/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(4, 13, 0)
#include_next <linux/build_bug.h>
#else
#include <linux/bug.h>

/* Linux 4.9.297 doesn't provide BUILD_BUG_ON anymore in linux/bug.h
 * also identified itself with the version number 4.9.255 when decoding the
 * LINUX_VERSION_CODE. So we have to try to guess now if we need to include
 * linux/build_bug.h based on whether BUILD_BUG_ON is defined  or not after
 * including linux/bug.h
 */
#ifndef BUILD_BUG_ON
#include_next <linux/build_bug.h>
#endif

#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_ */
