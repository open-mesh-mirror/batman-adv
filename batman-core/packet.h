/*
 * Copyright (C) 2007-2008 B.A.T.M.A.N. contributors:
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



#define ETH_P_BATMAN  0x0842

#define BAT_PACKET    0x01
#define BAT_ICMP      0x02
#define BAT_UNICAST   0x03
#define BAT_BCAST     0x04



#define ECHO_REPLY 0
#define DESTINATION_UNREACHABLE 3
#define ECHO_REQUEST 8
#define TTL_EXCEEDED 11



struct batman_packet
{
	uint8_t  packet_type;
	uint8_t  version;  /* batman version field */
	uint8_t  flags;    /* 0x80: UNIDIRECTIONAL link, 0x40: DIRECTLINK flag, ... */
	uint8_t  ttl;
	uint8_t  gwflags;  /* flags related to gateway functions: gateway class */
	uint8_t  tq;
	uint16_t seqno;
	uint8_t  orig[6];
	uint8_t  old_orig[6];
	uint8_t  num_hna;
	uint8_t  pad;
} __attribute__((packed));

struct icmp_packet
{
	uint8_t  packet_type;
	uint8_t  msg_type;   /* 0 = ECHO REPLY, 3 = DESTINATION_UNREACHABLE, 8 = ECHO_REQUEST, 11 = TTL exceeded */
	uint8_t  dst[6];
	uint8_t  orig[6];
	uint8_t  ttl;
	uint8_t  uid;
	uint16_t seqno;
} __attribute__((packed));

struct unicast_packet
{
	uint8_t  packet_type;
	uint8_t  ttl;
	uint8_t  dest[6];
} __attribute__((packed));

struct bcast_packet
{
	uint8_t  packet_type;
	uint8_t  padding;
	uint8_t  orig[6];
	uint16_t seqno;
} __attribute__((packed));

