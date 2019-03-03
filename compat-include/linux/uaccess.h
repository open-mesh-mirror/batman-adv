/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_UACCESS_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_UACCESS_H_

#include <linux/version.h>
#include_next <linux/uaccess.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)

static inline int batadv_access_ok(int type, const void __user *p,
				   unsigned long size)
{
	return access_ok(type, p, size);
}

#ifdef access_ok
#undef access_ok
#endif

#define access_ok_get(_1, _2, _3 , access_ok_name, ...) access_ok_name
#define access_ok(...) \
	access_ok_get(__VA_ARGS__, access_ok3, access_ok2)(__VA_ARGS__)

#define access_ok2(addr, size)	batadv_access_ok(VERIFY_WRITE, (addr), (size))
#define access_ok3(type, addr, size)	batadv_access_ok((type), (addr), (size))

#endif /* < KERNEL_VERSION(5, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_UACCESS_H_ */
