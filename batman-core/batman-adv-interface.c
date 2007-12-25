/*
 * Copyright (C) 2007 B.A.T.M.A.N. contributors:
 * Marek Lindner
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */





#include "batman-adv-main.h"
#include "batman-adv-interface.h"
#include "types.h"



#define BAT_IF_MTU (1500 - (sizeof(struct unicast_packet) > sizeof(struct bcast_packet) ? sizeof(struct unicast_packet) : sizeof(struct bcast_packet)))



void interface_setup(struct net_device *dev)
{
	struct bat_priv *priv = netdev_priv(dev);
	char dev_addr[ETH_ALEN];

	ether_setup(dev);

	dev->open = interface_open;
	dev->stop = interface_release;
	dev->get_stats = interface_stats;
	dev->change_mtu = interface_change_mtu;
	dev->hard_start_xmit = interface_tx;
	dev->destructor = free_netdev;
// 	dev->hard_header_cache = NULL;

	dev->features |= NETIF_F_NO_CSUM;
	dev->mtu = BAT_IF_MTU;

	/* generate random address */
	*(u16 *)dev_addr = htons(0x00FF);
	get_random_bytes(dev_addr + sizeof(u16), 4);
	memcpy(dev->dev_addr, dev_addr, sizeof(dev->dev_addr));

	memset(priv, 0, sizeof(struct bat_priv));
	spin_lock_init(&priv->lock);
}

int interface_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

int interface_release(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

struct net_device_stats *interface_stats(struct net_device *dev)
{
	struct bat_priv *priv = netdev_priv(dev);
	return &priv->stats;
}

int interface_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	struct bat_priv *priv = netdev_priv(dev);
	spinlock_t *lock = &priv->lock;

	/* check ranges */
	if ((new_mtu < 68) || (new_mtu > BAT_IF_MTU))
		return -EINVAL;

	spin_lock_irqsave(lock, flags);
	dev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);

	return 0;
}

int interface_tx(struct sk_buff *skb, struct net_device *dev)
{
// 	int len;
// 	char *data, shortpkt[ETH_ZLEN];
// 	struct bat_priv *priv = netdev_priv(dev);
// 	struct ethhdr *ethhdr;
// 	char src_str[ETH_STR_LEN], dst_str[ETH_STR_LEN];
//
// 	data = skb->data;
// 	len = skb->len;
//
// 	if (len < ETH_ZLEN) {
// 		memset(shortpkt, 0, ETH_ZLEN);
// 		memcpy(shortpkt, skb->data, skb->len);
// 		len = ETH_ZLEN;
// 		data = shortpkt;
// 	}
//
// 	dev->trans_start = jiffies;
//

	kfree_skb(skb);

	return 0;
}
