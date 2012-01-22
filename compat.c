/*
 * Copyright (C) 2007-2012 B.A.T.M.A.N. contributors:
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#include <linux/in.h>
#include <linux/version.h>
#include "main.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)

void free_rcu_gw_node(struct rcu_head *rcu)
{
	struct gw_node *gw_node;

	gw_node = container_of(rcu, struct gw_node, rcu);
	kfree(gw_node);
}

void free_rcu_neigh_node(struct rcu_head *rcu)
{
	struct neigh_node *neigh_node;

	neigh_node = container_of(rcu, struct neigh_node, rcu);
	kfree(neigh_node);
}

void free_rcu_tt_local_entry(struct rcu_head *rcu)
{
	struct tt_common_entry *tt_common_entry;
	struct tt_local_entry *tt_local_entry;

	tt_common_entry = container_of(rcu, struct tt_common_entry, rcu);
	tt_local_entry = container_of(tt_common_entry, struct tt_local_entry,
				      common);
	kfree(tt_local_entry);
}

#endif /* < KERNEL_VERSION(3, 0, 0) */
