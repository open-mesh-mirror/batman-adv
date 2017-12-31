#! /bin/sh
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2007-2018  B.A.T.M.A.N. contributors:
#
# Marek Lindner, Simon Wunderlich
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of version 2 of the GNU General Public
# License as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

set -e

# for kernel < 3.13 to make netlink compat code work
sed -i \
	-e 's/^static const struct genl_multicast_group batadv_netlink_mcgrps/static __genl_const struct genl_multicast_group batadv_netlink_mcgrps/' \
	-e 's/^static const struct genl_ops batadv_netlink_ops/static __genl_const struct genl_ops batadv_netlink_ops/' \
	build/net/batman-adv/netlink.c
