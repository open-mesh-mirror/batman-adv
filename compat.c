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

void free_rcu_softif_neigh(struct rcu_head *rcu)
{
	struct softif_neigh *softif_neigh;

	softif_neigh = container_of(rcu, struct softif_neigh, rcu);
	kfree(softif_neigh);
}

void free_rcu_tt_local_entry(struct rcu_head *rcu)
{
	struct tt_local_entry *tt_local_entry;

	tt_local_entry = container_of(rcu, struct tt_local_entry, rcu);
	kfree(tt_local_entry);
}

#endif /* < KERNEL_VERSION(3, 0, 0) */
