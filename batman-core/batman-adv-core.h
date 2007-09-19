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





#include <linux/module.h>	/* needed by all modules */
#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include <linux/netdevice.h>	/* netdevice */



/* Kernel Programming */
#define LINUX

#define DRIVER_AUTHOR "Marek Lindner <lindner_marek@yahoo.de>"
#define DRIVER_DESC   "B.A.T.M.A.N. Advanced"
#define DRIVER_DEVICE "batman-adv"

#define COMPAT_VERSION 1
#define TTL 50                /* Time To Live of broadcast messages */



int batman_attach_core(struct net_device *dev, u_int8_t *ie_buff, u_int8_t *ie_buff_len);
int batman_detach_core(struct net_device *dev);
void ogm_update_core(struct net_device *dev, u_int8_t *ie_buff, u_int8_t *ie_buff_len);

