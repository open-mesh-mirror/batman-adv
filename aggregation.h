/*
 * Copyright (C) 2007-2009 B.A.T.M.A.N. contributors:
 * Marek Lindner, Simon Wunderlich
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

#define aggregated_packet(buff_pos, packet_len, num_hna) \
		buff_pos + sizeof(struct batman_packet) + (num_hna * ETH_ALEN) <= packet_len && \
		buff_pos + sizeof(struct batman_packet) + (num_hna * ETH_ALEN) <= MAX_AGGREGATION_BYTES


void add_bat_packet_to_list(unsigned char *packet_buff, int packet_len, struct batman_if *if_outgoing, char own_packet, unsigned long send_time);
void receive_aggr_bat_packet(struct ethhdr *ethhdr, unsigned char *packet_buff, int packet_len, struct batman_if *if_incoming);
