/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_

#include <linux/version.h>
#include_next <linux/overflow.h>

#if LINUX_VERSION_IS_LESS(6, 1, 0)

#undef check_add_overflow
#define check_add_overflow(a, b, d)	\
	__must_check_overflow(__builtin_add_overflow(a, b, d))

#endif /* LINUX_VERSION_IS_LESS(6, 1, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_OVERFLOW_H_ */
