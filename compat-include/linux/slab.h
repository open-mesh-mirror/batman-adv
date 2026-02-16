/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_

#include <linux/version.h>
#include_next <linux/slab.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(7, 1, 0)

#define kzalloc_obj(P, GFP) \
	kzalloc(sizeof(P), GFP)

#define kmalloc_obj(P, GFP) \
	kmalloc(sizeof(P), GFP)

#define kmalloc_objs(P, COUNT, GFP) \
	kmalloc_array((COUNT), sizeof(P), GFP)

#endif /* < KERNEL_VERSION(7, 1, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_ */
