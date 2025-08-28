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

#if LINUX_VERSION_IS_LESS(5, 10, 0)
#include "net/genetlink.h"
#endif /* LINUX_VERSION_IS_LESS(5, 10, 0) */

#endif /* __KERNEL__ */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
