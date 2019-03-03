/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_FS_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_FS_H_

#include <linux/version.h>
#include_next <linux/fs.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)

static inline struct dentry *batadv_file_dentry(const struct file *file)
{
	struct dentry *dentry = file->f_path.dentry;

#ifdef DCACHE_OP_REAL
	if (unlikely(dentry->d_flags & DCACHE_OP_REAL))
		return dentry->d_op->d_real(dentry, file_inode(file));
#endif

	return dentry;
}

#define file_dentry batadv_file_dentry

#endif /* < KERNEL_VERSION(4, 6, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_FS_H_ */
