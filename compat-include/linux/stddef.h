/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_

#include <linux/version.h>
#include_next <linux/stddef.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)

#ifndef sizeof_member
#define sizeof_member(TYPE, MEMBER)	(sizeof(((TYPE *)0)->MEMBER))
#endif

#endif /* < KERNEL_VERSION(5, 5, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_ */
