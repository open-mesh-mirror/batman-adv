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

#ifndef _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_

#include <linux/version.h>
#include_next <net/genetlink.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)

#define genl_info_snd_portid(__genl_info) (__genl_info->snd_pid)

#else

#define genl_info_snd_portid(__genl_info) (__genl_info->snd_portid)

#endif /* < KERNEL_VERSION(3, 7, 0) */


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)

#include <linux/export.h>

struct batadv_genl_family {
	/* data handled by the actual kernel */
	struct genl_family family;

	/* data which has to be copied to family by
	 * batadv_genlmsg_multicast_netns
	 */
	unsigned int		id;
	unsigned int		hdrsize;
	char			name[GENL_NAMSIZ];
	unsigned int		version;
	unsigned int		maxattr;
	bool			netnsok;
	bool			parallel_ops;
	int			(*pre_doit)(struct genl_ops *ops,
					    struct sk_buff *skb,
					    struct genl_info *info);
	void			(*post_doit)(struct genl_ops *ops,
					     struct sk_buff *skb,
					     struct genl_info *info);
	/* WARNING not supported
	 * int			(*mcast_bind)(struct net *net, int group);
	 * void			(*mcast_unbind)(struct net *net, int group);
	 */
	struct nlattr		**attrbuf;	/* private */
	struct genl_ops		*ops;		/* private */
	struct genl_multicast_group *mcgrps; /* private */
	unsigned int		n_ops;		/* private */
	unsigned int		n_mcgrps;	/* private */
	/* unsigned int		mcgrp_offset;	private, WARNING unsupported */
	struct list_head	family_list;	/* private */
	struct module		*module;
};

#define genl_family batadv_genl_family

#define genlmsg_multicast_netns batadv_genlmsg_multicast_netns

static inline int
batadv_genlmsg_multicast_netns(struct batadv_genl_family *family,
			       struct net *net,
			       struct sk_buff *skb,
			       u32 portid, unsigned int group,
			       gfp_t flags)
{
	group = family->mcgrps[group].id;
	return nlmsg_multicast(
		net->genl_sock,
		skb, portid, group, flags);
}

#define genlmsg_put(_skb, _pid, _seq, _family, _flags, _cmd) \
	genlmsg_put(_skb, _pid, _seq, &(_family)->family, _flags, _cmd)

#define genl_unregister_family(_family) \
	genl_unregister_family(&(_family)->family)

#define genl_register_family_with_ops_groups(family, ops, grps) \
	batadv_genl_register_family_with_ops_grps((family), \
						  (ops), ARRAY_SIZE(ops), \
						  (grps), ARRAY_SIZE(grps))

static inline int batadv_genl_register_family(struct genl_family *family)
{
	unsigned int i;
	int ret;

	family->family.id = family->id;
	family->family.hdrsize = family->hdrsize;
	strncpy(family->family.name, family->name, sizeof(family->family.name));
	family->family.version = family->version;
	family->family.maxattr = family->maxattr;
	family->family.netnsok = family->netnsok;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
	family->family.parallel_ops = family->parallel_ops;
#endif
	family->family.pre_doit = family->pre_doit;
	family->family.post_doit = family->post_doit;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	family->family.module = family->module;
#endif

	ret = genl_register_family(&family->family);
	if (ret < 0)
		return ret;

	family->attrbuf = family->family.attrbuf;
	family->id = family->family.id;

	for (i = 0; i < family->n_ops; i++) {
		ret = genl_register_ops(&family->family, &family->ops[i]);
		if (ret < 0)
			goto err;
	}

	for (i = 0; i < family->n_mcgrps; i++) {
		ret = genl_register_mc_group(&family->family,
					     &family->mcgrps[i]);
		if (ret)
			goto err;
	}

	return 0;

 err:
	genl_unregister_family(family);
	return ret;
}

static inline int
batadv_genl_register_family_with_ops_grps(struct genl_family *family,
					  struct genl_ops *ops, size_t n_ops,
					  struct genl_multicast_group *mcgrps,
					  size_t n_mcgrps)
{
	family->ops = ops;
	family->n_ops = n_ops;
	family->mcgrps = mcgrps;
	family->n_mcgrps = n_mcgrps;
	family->module = THIS_MODULE;

	return batadv_genl_register_family(family);
}

#define __genl_const

#else

#define __genl_const const

#endif /* < KERNEL_VERSION(3, 13, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_ */
