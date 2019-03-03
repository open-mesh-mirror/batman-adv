/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_

#include <linux/version.h>
#include_next <net/genetlink.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)

static inline
void batadv_genl_dump_check_consistent(struct netlink_callback *cb,
				       void *user_hdr)
{
	struct genl_family genl_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &genl_family);
}

#define genl_dump_check_consistent batadv_genl_dump_check_consistent

#endif /* < KERNEL_VERSION(4, 15, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_ */
