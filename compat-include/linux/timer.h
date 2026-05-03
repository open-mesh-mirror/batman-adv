/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_

#include <linux/version.h>
#include_next <linux/timer.h>

#if LINUX_VERSION_IS_LESS(6, 2, 0) && \
    !(LINUX_VERSION_IS_GEQ(5, 15, 200) && LINUX_VERSION_IS_LESS(5, 16, 0)) && \
    !(LINUX_VERSION_IS_GEQ(6, 1, 158) && LINUX_VERSION_IS_LESS(6, 2, 0))

#define timer_delete_sync(_timer) del_timer_sync(_timer)
#define timer_delete(_timer) del_timer(_timer)

static inline int timer_shutdown_sync(struct timer_list *timer)
{
	return del_timer_sync(timer);
}

#endif /* LINUX_VERSION_IS_LESS(6, 2, 0) */

#if LINUX_VERSION_IS_LESS(6, 16, 0)

#define timer_container_of(var, callback_timer, timer_fieldname)	\
	from_timer(var, callback_timer, timer_fieldname)

#endif /* LINUX_VERSION_IS_LESS(6, 16, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_TIMER_H_ */
