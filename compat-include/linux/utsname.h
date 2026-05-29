/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_UTSNAME_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_UTSNAME_H_

#include_next <linux/utsname.h>

#define init_utsname() batadv_init_utsname()

extern struct new_utsname batadv_version_name;

static inline struct new_utsname *batadv_init_utsname(void)
{
	return &batadv_version_name;
}

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_UTSNAME_H_ */
