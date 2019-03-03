/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SEQ_FILE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SEQ_FILE_H_

#include <linux/version.h>
#include_next <linux/seq_file.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)

static inline bool seq_has_overflowed(struct seq_file *m)
{
	return m->count == m->size;
}

#endif /* < KERNEL_VERSION(3, 19, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SEQ_FILE_H_ */
