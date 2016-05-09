/* Copyright (C) 2016 B.A.T.M.A.N. contributors:
 *
 * Matthias Schiffer
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
 */

#ifndef _UAPI_LINUX_BATMAN_ADV_H_
#define _UAPI_LINUX_BATMAN_ADV_H_

#define BATADV_NL_NAME "batadv"

/**
 * enum batadv_nl_attrs - batman-adv netlink attributes
 *
 * @BATADV_ATTR_UNSPEC: unspecified attribute to catch errors
 * @__BATADV_ATTR_AFTER_LAST: internal use
 * @NUM_BATADV_ATTR: total number of batadv_nl_attrs available
 * @BATADV_ATTR_MAX: highest attribute number currently defined
 */
enum batadv_nl_attrs {
	BATADV_ATTR_UNSPEC,
	/* add attributes above here, update the policy in netlink.c */
	__BATADV_ATTR_AFTER_LAST,
	NUM_BATADV_ATTR = __BATADV_ATTR_AFTER_LAST,
	BATADV_ATTR_MAX = __BATADV_ATTR_AFTER_LAST - 1
};

/**
 * enum batadv_nl_commands - supported batman-adv netlink commands
 *
 * @BATADV_CMD_UNSPEC: unspecified command to catch errors
 * @__BATADV_CMD_AFTER_LAST: internal use
 * @BATADV_CMD_MAX: highest used command number
 */
enum batadv_nl_commands {
	BATADV_CMD_UNSPEC,
	/* add new commands above here */
	__BATADV_CMD_AFTER_LAST,
	BATADV_CMD_MAX = __BATADV_CMD_AFTER_LAST - 1
};

#endif /* _UAPI_LINUX_BATMAN_ADV_H_ */
