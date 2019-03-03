/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_LIST_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_LIST_H_

#include <linux/version.h>
#include_next <linux/list.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)

#define hlist_add_behind(n, prev) hlist_add_after(prev, n)

#endif /* < KERNEL_VERSION(3, 17, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

static inline bool hlist_fake(struct hlist_node *h)
{
	return h->pprev == &h->next;
}

#endif /* < KERNEL_VERSION(4, 3, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_LIST_H_ */
