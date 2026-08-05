/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_

#include <linux/version.h>
#include_next <linux/compiler.h>

#if LINUX_VERSION_IS_LESS(6, 5, 0)

#define __cleanup(func)			__attribute__((__cleanup__(func)))

#endif /* LINUX_VERSION_IS_LESS(6, 5, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_ */
