/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2020  B.A.T.M.A.N. contributors:
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

#if LINUX_VERSION_IS_LESS(4, 13, 0)

#define batadv_softif_validate(__tb, __data, __extack) \
	batadv_softif_validate(__tb, __data)

#define batadv_softif_newlink(__src_net, __dev, __tb, __data, __extack) \
	batadv_softif_newlink(__src_net, __dev, __tb, __data)

#endif /* LINUX_VERSION_IS_LESS(4, 13, 0) */


#if LINUX_VERSION_IS_LESS(4, 15, 0)

#define batadv_softif_slave_add(__dev, __slave_dev, __extack) \
	batadv_softif_slave_add(__dev, __slave_dev)

#endif /* LINUX_VERSION_IS_LESS(4, 15, 0) */

#endif /* __KERNEL__ */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
