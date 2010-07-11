/*
 * Copyright (C) 2010 B.A.T.M.A.N. contributors:
 *
 * Andreas Langer
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
 */
#include "main.h"
#include "fragmentation.h"


struct sk_buff *merge_frag_packet(struct list_head *head,
				  struct frag_packet_list_entry *tfp,
				  struct sk_buff *skb)
{
	struct unicast_frag_packet *up =
		(struct unicast_frag_packet *) skb->data;
	struct sk_buff *tmp_skb;

	/* set skb to the first part and tmp_skb to the second part */
	if (up->flags & UNI_FRAG_HEAD) {
		tmp_skb = tfp->skb;
	} else {
		tmp_skb = skb;
		skb = tfp->skb;
	}

	skb_pull(tmp_skb, sizeof(struct unicast_frag_packet));
	if (pskb_expand_head(skb, 0, tmp_skb->len, GFP_ATOMIC) < 0) {
		/* free buffered skb, skb will be freed later */
		kfree_skb(tfp->skb);
		return NULL;
	}

	/* move free entry to end */
	tfp->skb = NULL;
	tfp->seqno = 0;
	list_move_tail(&tfp->list, head);

	memcpy(skb_put(skb, tmp_skb->len), tmp_skb->data, tmp_skb->len);
	kfree_skb(tmp_skb);
	return skb;
}

void create_frag_entry(struct list_head *head, struct sk_buff *skb)
{
	struct frag_packet_list_entry *tfp;
	struct unicast_frag_packet *up =
		(struct unicast_frag_packet *) skb->data;

	/* free and oldest packets stand at the end */
	tfp = list_entry((head)->prev, typeof(*tfp), list);
	kfree_skb(tfp->skb);

	tfp->seqno = ntohs(up->seqno);
	tfp->skb = skb;
	list_move(&tfp->list, head);
	return;
}

void create_frag_buffer(struct list_head *head)
{
	int i;
	struct frag_packet_list_entry *tfp;

	for (i = 0; i < FRAG_BUFFER_SIZE; i++) {
		tfp = kmalloc(sizeof(struct frag_packet_list_entry),
			GFP_ATOMIC);
		tfp->skb = NULL;
		tfp->seqno = 0;
		INIT_LIST_HEAD(&tfp->list);
		list_add(&tfp->list, head);
	}

	return;
}

struct frag_packet_list_entry *search_frag_packet(struct list_head *head,
						 struct unicast_frag_packet *up)
{
	struct frag_packet_list_entry *tfp;
	struct unicast_frag_packet *tmp_up = NULL;
	uint16_t search_seqno;

	if (up->flags & UNI_FRAG_HEAD)
		search_seqno = ntohs(up->seqno)+1;
	else
		search_seqno = ntohs(up->seqno)-1;

	list_for_each_entry(tfp, head, list) {

		if (!tfp->skb)
			continue;

		if (tfp->seqno == ntohs(up->seqno))
			goto mov_tail;

		tmp_up = (struct unicast_frag_packet *) tfp->skb->data;

		if (tfp->seqno == search_seqno) {

			if ((tmp_up->flags & UNI_FRAG_HEAD) !=
			    (up->flags & UNI_FRAG_HEAD))
				return tfp;
			else
				goto mov_tail;
		}
	}
	return NULL;

mov_tail:
	list_move_tail(&tfp->list, head);
	return NULL;
}

void frag_list_free(struct list_head *head)
{
	struct frag_packet_list_entry *pf, *tmp_pf;

	if (!list_empty(head)) {

		list_for_each_entry_safe(pf, tmp_pf, head, list) {
			kfree_skb(pf->skb);
			list_del(&pf->list);
			kfree(pf);
		}
	}
	return;
}
