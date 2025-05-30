/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_UNALIGNED_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_UNALIGNED_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(6, 12, 0)
#include_next <linux/unaligned.h>
#else
#include <asm/unaligned.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_UNALIGNED_H_ */
