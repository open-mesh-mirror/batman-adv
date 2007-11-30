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
#include "batman-adv-routing.h"
#include "batman-adv-log.h"
#include "types.h"



DECLARE_WAIT_QUEUE_HEAD(thread_wait);

atomic_t data_ready_cond;



void receive_bat_packet(struct ethhdr *ethhdr, struct batman_packet *batman_packet, unsigned char *hna_buff, int hna_buff_len, struct batman_if *batman_if)
{
	// do something here !!!
}

int packet_recv_thread(void *data)
{
	struct list_head *list_pos;
	struct batman_if *batman_if;
	struct kvec iov;
	struct msghdr msg;
	struct ethhdr *ethhdr;
	struct batman_packet *batman_packet;
	unsigned char packet_buff[2000];
	unsigned int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
	int result;

	iov.iov_base = packet_buff;
	iov.iov_len = sizeof(packet_buff);
	msg.msg_flags = flags;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;

	atomic_set(&data_ready_cond, 0);

	while (!kthread_should_stop()) {

		wait_event_interruptible(thread_wait, atomic_read(&data_ready_cond));

		if (kthread_should_stop())
			break;

		list_for_each(list_pos, &if_list) {
			batman_if = list_entry(list_pos, struct batman_if, list);

			while ((result = kernel_recvmsg(batman_if->raw_sock, &msg, &iov, 1, sizeof(packet_buff), flags)) > 0) {

				if (result < sizeof(struct ethhdr) + 1)
					continue;

				ethhdr = (struct ethhdr *)packet_buff;
				batman_packet = (struct batman_packet *)(packet_buff + sizeof(struct ethhdr));

				/* batman packet */
				switch (batman_packet->packet_type) {

					case BAT_PACKET:

						/* drop packet if it has no batman packet payload */
						if (result < sizeof(struct ethhdr) + sizeof(struct batman_packet) )
							continue;

						/* network to host order for our 16bit seqno. */
						batman_packet->seqno = ntohs(batman_packet->seqno);

						receive_bat_packet(ethhdr, batman_packet, packet_buff + sizeof(struct ethhdr) + sizeof(struct batman_packet), result, batman_if);
						break;

					/* unicast packet */
					case BAT_UNICAST:

						break;

					/* batman icmp packet */
					case BAT_ICMP:

						break;

					/* broadcast packet */
					case BAT_BCAST:

						break;

				}

			}

			if (result < 0)
				debug_log(LOG_TYPE_CRIT, "batman-adv: Could not receive packet from interface %s: %i\n", batman_if->net_dev->name, result);
		}

		printk("packet_recv_thread: thread woke up\n");
		atomic_set(&data_ready_cond, 0);
	}

	return 0;
}

void batman_data_ready(struct sock *sk, int len)
{
	void (*data_ready)(struct sock *, int) = sk->sk_user_data;

	data_ready(sk, len);

	atomic_set(&data_ready_cond, 1);
	wake_up_interruptible(&thread_wait);
}

