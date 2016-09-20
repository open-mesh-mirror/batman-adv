#ifndef _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_
#define _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_

#include <linux/version.h>
#include_next <net/cfg80211.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)

#if !IS_ENABLED(CONFIG_CFG80211)

#define cfg80211_get_station(dev, mac_addr, sinfo) \
	batadv_cfg80211_get_station(dev, mac_addr, sinfo)

static inline int batadv_cfg80211_get_station(struct net_device *dev,
					      const u8 *mac_addr,
					      struct station_info *sinfo)
{
	return -ENOENT;
}
#endif

#endif /* < KERNEL_VERSION(4, 8, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_ */
