/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_CONTAINER_OF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_CONTAINER_OF_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(5, 16, 0)
#include_next <linux/container_of.h>
#else
#include <linux/kernel.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CONTAINER_OF_H_ */
