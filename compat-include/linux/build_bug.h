/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
#include_next <linux/build_bug.h>
#else
#include <linux/bug.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_BUILD_BUG_H_ */
