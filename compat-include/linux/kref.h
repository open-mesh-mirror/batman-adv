/* Copyright (C) 2007-2017  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_KREF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_KREF_H_

#include <linux/version.h>
#include_next <linux/kref.h>

#include <linux/atomic.h>
#include <linux/kernel.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)

/* some stable versions like Linux 3.2.44 also introduced this function
 * and would therefore break the build because they trigger a redefinition
 * of this function. Instead rename this function to be in the batadv_*
 * namespace
 */
#define kref_get_unless_zero(__kref) batadv_kref_get_unless_zero(__kref)

static inline int __must_check batadv_kref_get_unless_zero(struct kref *kref)
{
	return atomic_add_unless(&kref->refcount, 1, 0);
}

#endif /* < KERNEL_VERSION(3, 8, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_KREF_H_ */
