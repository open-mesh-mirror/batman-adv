/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_

#include <linux/version.h>
#include_next <linux/overflow.h>

#if LINUX_VERSION_IS_LESS(5, 8, 0)

static inline size_t __must_check batadv_size_mul(size_t f1, size_t f2)
{
	size_t bytes;

	if (check_mul_overflow(f1, f2, &bytes))
		return SIZE_MAX;

	return bytes;
}

#ifndef flex_array_size
#define flex_array_size(p, member, count)				\
	__builtin_choose_expr(__is_constexpr(count),			\
		(count) * sizeof(*(p)->member) + __must_be_array((p)->member),	\
		batadv_size_mul(count, sizeof(*(p)->member) + __must_be_array((p)->member)))
#endif /* flex_array_size */

#endif /* LINUX_VERSION_IS_LESS(5, 8, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_ */
