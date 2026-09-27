/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_

#include <linux/version.h>
#include_next <linux/slab.h>

#if LINUX_VERSION_IS_LESS(7, 0, 0)

#include <linux/gfp.h>

#ifndef kzalloc_obj
#define kzalloc_obj(P, ...) \
	kzalloc(sizeof(P), default_gfp(__VA_ARGS__))
#endif /* kzalloc_obj */

#ifndef kmalloc_obj
#define kmalloc_obj(P, ...) \
	kmalloc(sizeof(P), default_gfp(__VA_ARGS__))
#endif /* kmalloc_obj */

#ifndef kmalloc_objs
#define kmalloc_objs(P, COUNT, ...) \
	kmalloc_array((COUNT), sizeof(P), default_gfp(__VA_ARGS__))
#endif /* kmalloc_objs */

#endif /* < KERNEL_VERSION(7, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SLAB_H_ */
