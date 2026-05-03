/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SPRINTF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SPRINTF_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(6, 6, 0)
#include_next <linux/sprintf.h>
#else
#include <linux/kernel.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_SPRINTF_H_ */
