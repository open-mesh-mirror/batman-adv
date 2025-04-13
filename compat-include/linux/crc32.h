/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_CRC32_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_CRC32_H_

#include <linux/version.h>
#include_next <linux/crc32.h>


#if LINUX_VERSION_IS_LESS(6, 15, 0)
#include <linux/crc32c.h>
#endif /* LINUX_VERSION_IS_LESS(6, 15, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CRC32_H_ */
