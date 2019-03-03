/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_

#include <linux/version.h>
#include_next <linux/compiler.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)

#ifndef READ_ONCE
#define READ_ONCE(x) ACCESS_ONCE(x)
#endif

#ifndef WRITE_ONCE
#define WRITE_ONCE(x, val) ({ \
	ACCESS_ONCE(x) = (val); \
})
#endif

#endif /* < KERNEL_VERSION(3, 19, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_ */
