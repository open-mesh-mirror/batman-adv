/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_CLEANUP_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_CLEANUP_H_

#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(6, 5, 0) && \
    !(LINUX_VERSION_IS_GEQ(6, 1, 79) && LINUX_VERSION_IS_LESS(6, 2, 0))

#include <linux/compiler.h>

#define CLASS(_name, var)						\
	class_##_name##_t var __cleanup(class_##_name##_destructor) =	\
		class_##_name##_constructor

#define scoped_guard(_name, args...)					\
	for (CLASS(_name, scope)(args),					\
	     *done = NULL; !done; done = (void *)1)

#define __DEFINE_UNLOCK_GUARD(_name, _type, _unlock, ...)		\
typedef struct {							\
	_type *lock;							\
	__VA_ARGS__;							\
} class_##_name##_t;							\
									\
static inline void class_##_name##_destructor(class_##_name##_t *_T)	\
{									\
	if (_T->lock) { _unlock; }					\
}

#define __DEFINE_LOCK_GUARD_1(_name, _type, _lock)			\
static inline class_##_name##_t class_##_name##_constructor(_type *l)	\
{									\
	class_##_name##_t _t = { .lock = l }, *_T = &_t;		\
	_lock;								\
	return _t;							\
}

#define DEFINE_LOCK_GUARD_1(_name, _type, _lock, _unlock, ...)		\
__DEFINE_UNLOCK_GUARD(_name, _type, _unlock, __VA_ARGS__)		\
__DEFINE_LOCK_GUARD_1(_name, _type, _lock)

#else
#include_next <linux/cleanup.h>
#endif /* LINUX_VERSION_IS_LESS(6, 5, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_CLEANUP_H_ */
