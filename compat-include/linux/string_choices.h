/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_STRING_CHOICES_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_STRING_CHOICES_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(6, 5, 0)
#include_next <linux/string_choices.h>
#elif LINUX_VERSION_IS_GEQ(5, 18, 0)
#include <linux/string_helpers.h>
#else

static inline const char *str_yes_no(bool v)
{
	return v ? "yes" : "no";
}

static inline const char *str_on_off(bool v)
{
	return v ? "on" : "off";
}

#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_STRING_CHOICES_H_ */
