/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 */

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#ifdef __KERNEL__

#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include <linux/kconfig.h>
#include <generated/autoconf.h>

#include "compat-autoconf.h"


#if LINUX_VERSION_IS_LESS(6, 4, 0)

#if IS_ENABLED(CONFIG_SLOB)
#error kfree_rcu for kmem_cache not supported when SLOB is enabled
#endif

#endif /* LINUX_VERSION_IS_LESS(6, 4, 0) */

#endif /* __KERNEL__ */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
