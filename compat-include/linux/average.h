/* Copyright (C) 2007-2016 B.A.T.M.A.N. contributors:
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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_AVERAGE_H
#define _NET_BATMAN_ADV_COMPAT_LINUX_AVERAGE_H

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
#include_next <linux/average.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

/* Exponentially weighted moving average (EWMA) */

#define DECLARE_EWMA(name, _factor, _weight)				\
	struct ewma_##name {						\
		unsigned long internal;					\
	};								\
	static inline void ewma_##name##_init(struct ewma_##name *e)	\
	{								\
		BUILD_BUG_ON(!__builtin_constant_p(_factor));		\
		BUILD_BUG_ON(!__builtin_constant_p(_weight));		\
		BUILD_BUG_ON_NOT_POWER_OF_2(_factor);			\
		BUILD_BUG_ON_NOT_POWER_OF_2(_weight);			\
		e->internal = 0;					\
	}								\
	static inline unsigned long					\
	ewma_##name##_read(struct ewma_##name *e)			\
	{								\
		BUILD_BUG_ON(!__builtin_constant_p(_factor));		\
		BUILD_BUG_ON(!__builtin_constant_p(_weight));		\
		BUILD_BUG_ON_NOT_POWER_OF_2(_factor);			\
		BUILD_BUG_ON_NOT_POWER_OF_2(_weight);			\
		return e->internal >> ilog2(_factor);			\
	}								\
	static inline void ewma_##name##_add(struct ewma_##name *e,	\
					     unsigned long val)		\
	{								\
		unsigned long internal = ACCESS_ONCE(e->internal);	\
		unsigned long weight = ilog2(_weight);			\
		unsigned long factor = ilog2(_factor);			\
									\
		BUILD_BUG_ON(!__builtin_constant_p(_factor));		\
		BUILD_BUG_ON(!__builtin_constant_p(_weight));		\
		BUILD_BUG_ON_NOT_POWER_OF_2(_factor);			\
		BUILD_BUG_ON_NOT_POWER_OF_2(_weight);			\
									\
		ACCESS_ONCE(e->internal) = internal ?			\
			(((internal << weight) - internal) +		\
				(val << factor)) >> weight :		\
			(val << factor);				\
	}

#endif /* < KERNEL_VERSION(4, 3, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_AVERAGE_H */
