/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_VERSION_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_VERSION_H_

#include_next <linux/version.h>

#define LINUX_VERSION_IS_LESS(a, b, c) \
	(LINUX_VERSION_CODE < KERNEL_VERSION(a, b, c))

#define LINUX_VERSION_IS_GEQ(a, b, c) \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(a, b, c))

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_VERSION_H_ */
