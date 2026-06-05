/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_WORKQUEUE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_WORKQUEUE_H_

#include <linux/version.h>
#include_next <linux/workqueue.h>

#if LINUX_VERSION_IS_LESS(6, 10, 0)

static inline bool disable_work_sync(struct work_struct *work)
{
	/* don't disable anything because kernel doesn't support it */
	return cancel_work_sync(work);
}

static inline bool disable_delayed_work(struct delayed_work *dwork)
{
	/* don't disable anything because kernel doesn't support it */
	return cancel_delayed_work(dwork);
}

static inline bool disable_delayed_work_sync(struct delayed_work *dwork)
{
	/* don't disable anything because kernel doesn't support it */
	return cancel_delayed_work_sync(dwork);
}

static inline bool enable_delayed_work(struct delayed_work *dwork)
{
	/* do nothing - as disable compat also doesn't enable anything */
	return true;
}

#endif /* LINUX_VERSION_IS_LESS(6, 10, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_WORKQUEUE_ */
