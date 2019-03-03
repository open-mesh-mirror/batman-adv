/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_EVENTPOLL_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_EVENTPOLL_H_

#include <linux/version.h>
#include_next <linux/eventpoll.h>

#include <uapi/linux/eventpoll.h>

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_EVENTPOLL_H_ */
