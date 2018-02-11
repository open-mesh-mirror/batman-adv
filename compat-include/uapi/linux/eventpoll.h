/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007-2018  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_
#define _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_

#include <linux/version.h>
#include <linux/types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
#include_next <uapi/linux/eventpoll.h>
#else
#include <linux/eventpoll.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)

#define EPOLLIN		(__force __poll_t)0x00000001
#define EPOLLPRI	(__force __poll_t)0x00000002
#define EPOLLOUT	(__force __poll_t)0x00000004
#define EPOLLERR	(__force __poll_t)0x00000008
#define EPOLLHUP	(__force __poll_t)0x00000010
#define EPOLLNVAL	(__force __poll_t)0x00000020
#define EPOLLRDNORM	(__force __poll_t)0x00000040
#define EPOLLRDBAND	(__force __poll_t)0x00000080
#define EPOLLWRNORM	(__force __poll_t)0x00000100
#define EPOLLWRBAND	(__force __poll_t)0x00000200
#define EPOLLMSG	(__force __poll_t)0x00000400
#define EPOLLRDHUP	(__force __poll_t)0x00002000

#endif /* < KERNEL_VERSION(4, 12, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_UAPI_LINUX_EVENTPOLL_H_ */
