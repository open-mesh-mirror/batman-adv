/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_TYPES_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_TYPES_H_

#include <linux/version.h>
#include_next <linux/compiler_types.h>

#if LINUX_VERSION_IS_LESS(6, 10, 0)

#define __counted_by_be(member)

#endif /* LINUX_VERSION_IS_LESS(6, 10, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_TYPES_H_ */
