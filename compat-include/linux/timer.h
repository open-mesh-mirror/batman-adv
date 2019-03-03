/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H
#define _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H

#include <linux/version.h>
#include_next <linux/timer.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#define TIMER_DATA_TYPE		unsigned long
#define TIMER_FUNC_TYPE		void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
			       void (*callback)(struct timer_list *),
			       unsigned int flags)
{
	__setup_timer(timer, (TIMER_FUNC_TYPE)callback,
		      (TIMER_DATA_TYPE)timer, flags);
}

#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)

#endif /* < KERNEL_VERSION(4, 14, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H */
