/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_

#include <linux/version.h>
#include_next <linux/timer.h>

#if LINUX_VERSION_IS_LESS(6, 2, 0)

#define timer_delete_sync(_timer) del_timer_sync(_timer)
#define timer_delete(_timer) del_timer(_timer)

#endif /* LINUX_VERSION_IS_LESS(6, 2, 0) */

#if LINUX_VERSION_IS_LESS(6, 16, 0)

#define timer_container_of(var, callback_timer, timer_fieldname)	\
	from_timer(var, callback_timer, timer_fieldname)

#endif /* LINUX_VERSION_IS_LESS(6, 16, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_ */
