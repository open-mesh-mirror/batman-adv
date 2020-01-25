/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2015  Chris Metcalf <cmetcalf@ezchip.com>
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_STRING_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_STRING_H_

#include <linux/version.h>
#include_next <linux/string.h>
#include <asm-generic/errno-base.h>

#if LINUX_VERSION_IS_LESS(4, 3, 0)

#ifndef __HAVE_ARCH_STRSCPY

static inline ssize_t batadv_strscpy(char *dest, const char *src, size_t count)
{
	long res = 0;

	if (count == 0)
		return -E2BIG;

	while (count) {
		char c;

		c = src[res];
		dest[res] = c;
		if (!c)
			return res;
		res++;
		count--;
	}

	/* Hit buffer length without finding a NUL; force NUL-termination. */
	if (res)
		dest[res-1] = '\0';

	return -E2BIG;
}

#define strscpy(_dest, _src, _count) \
	batadv_strscpy((_dest), (_src), (_count))

#endif

#endif /* LINUX_VERSION_IS_LESS(4, 3, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_STRING_H_ */
