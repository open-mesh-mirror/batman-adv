/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_LOCKDEP_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_LOCKDEP_H_

#include <linux/version.h>
#include_next <linux/lockdep.h>

#if LINUX_VERSION_IS_LESS(5, 13, 0)

#ifdef CONFIG_LOCKDEP

#define lockdep_assert_not_held(l)	do {				\
		WARN_ON(debug_locks &&					\
			lockdep_is_held(l) == LOCK_STATE_HELD);		\
	} while (0)

#else /* !CONFIG_LOCKDEP */

#define lockdep_assert_not_held(l)		do { (void)(l); } while (0)

#endif /* !CONFIG_LOCKDEP */

#endif /* LINUX_VERSION_IS_LESS(5, 13, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_LOCKDEP_H_ */
