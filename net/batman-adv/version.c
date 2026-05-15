// SPDX-License-Identifier: GPL-2.0

#include "version.h"

#include <generated/utsrelease.h>
#include <linux/module.h>

#ifdef CONFIG_BATMAN_ADV_IN_TREE // UGLY_HACK_NEW
#define BATADV_SOURCE_VERSION "linux-" UTS_RELEASE
#else // UGLY_HACK_OLD

/* prefer version provided by Makefile */
#ifndef BATADV_SOURCE_VERSION
#define BATADV_SOURCE_VERSION "2026.2"
#endif

#endif // UGLY_HACK_STOP

const char *batadv_version = BATADV_SOURCE_VERSION;

/* WARNING userspace tools like batctl were relying on
 * /sys/module/batman_adv/version to check if the module was loaded. If it
 * isn't present, they usually error out before finishing setup of the batadv
 * interface. It should be kept until it is unlikely that there are active
 * installations of these "broken" versions of these tools with recent kernels.
 */
MODULE_VERSION(BATADV_SOURCE_VERSION);
