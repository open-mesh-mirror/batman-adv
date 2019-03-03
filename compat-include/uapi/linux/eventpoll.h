/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_
#define _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_

#include <linux/version.h>
#include <linux/types.h>
#include_next <uapi/linux/eventpoll.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)

#ifndef EPOLLIN
#define EPOLLIN		(__force __poll_t)0x00000001
#endif

#ifndef EPOLLRDNORM
#define EPOLLRDNORM	(__force __poll_t)0x00000040
#endif

#endif /* < KERNEL_VERSION(4, 12, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_ */
