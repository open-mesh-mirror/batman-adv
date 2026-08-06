/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_

#include <linux/version.h>
#include_next <linux/slab.h>

#if LINUX_VERSION_IS_LESS(7, 0, 0)

#ifndef kzalloc_obj
#define kzalloc_obj(P, GFP) \
	kzalloc(sizeof(P), GFP)
#endif /* kzalloc_obj */

#ifndef kmalloc_obj
#define kmalloc_obj(P, GFP) \
	kmalloc(sizeof(P), GFP)
#endif /* kmalloc_obj */

#ifndef kmalloc_objs
#define kmalloc_objs(P, COUNT, GFP) \
	kmalloc_array((COUNT), sizeof(P), GFP)
#endif /* kmalloc_objs */

#endif /* < KERNEL_VERSION(7, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_ */
