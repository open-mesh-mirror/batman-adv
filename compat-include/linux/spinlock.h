/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SPINLOCK_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SPINLOCK_H_

#include <linux/version.h>
#include_next <linux/spinlock.h>

#if LINUX_VERSION_IS_LESS(6, 15, 0) && \
    !(LINUX_VERSION_IS_GEQ(6, 12, 37) && LINUX_VERSION_IS_LESS(6, 13, 0))

#include <linux/cleanup.h>

DEFINE_LOCK_GUARD_1(spinlock_bh, spinlock_t,
		    spin_lock_bh(_T->lock),
		    spin_unlock_bh(_T->lock))

#endif /* LINUX_VERSION_IS_LESS(6, 15, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_SPINLOCK_H_ */
