/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_GFP_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_GFP_H_

#include <linux/version.h>
#include_next <linux/gfp.h>

#if LINUX_VERSION_IS_LESS(7, 0, 0)

#ifndef __default_gfp
/* Helper macro to avoid gfp flags if they are the default one */
#define __default_gfp(a,b,...) b
#define default_gfp(...) __default_gfp(,##__VA_ARGS__,GFP_KERNEL)
#endif

#endif /* < KERNEL_VERSION(7, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_GFP_H_ */
