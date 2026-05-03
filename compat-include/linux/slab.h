/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_

#include <linux/version.h>
#include_next <linux/slab.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(7, 0, 0)

#define kzalloc_obj(P, GFP) \
	kzalloc(sizeof(P), GFP)

#define kmalloc_obj(P, GFP) \
	kmalloc(sizeof(P), GFP)

#define kmalloc_objs(P, COUNT, GFP) \
	kmalloc_array((COUNT), sizeof(P), GFP)

#endif /* < KERNEL_VERSION(7, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_ */
