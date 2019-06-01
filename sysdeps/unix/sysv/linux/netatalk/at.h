/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _NETATALK_AT_H
#define _NETATALK_AT_H 1

#include <features.h>
#include <bits/types.h>
#include <bits/sockaddr.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

/* Constants from linux/atalk.h as of kernel version 5.0.  */

#define ATPORT_FIRST	1
#define ATPORT_RESERVED	128
#define ATPORT_LAST	254	/* 254 is only legal on localtalk */
#define ATADDR_ANYNET	0
#define ATADDR_ANYNODE	0
#define ATADDR_ANYPORT  0
#define ATADDR_BCAST	255
#define DDP_MAXSZ	587
#define DDP_MAXHOPS     15	/* 4 bits of hop counter */
#define SOL_ATALK       258     /* sockopt level for atalk */
#define SIOCATALKDIFADDR (SIOCPROTOPRIVATE + 0)

struct atalk_addr
{
  __uint16_t	s_net;  /* network byte order */
  __uint8_t	s_node;
};

struct atalk_netrange
{
  __uint8_t	nr_phase;
  __uint16_t	nr_firstnet; /* network byte order */
  __uint16_t	nr_lastnet;  /* network byte order */
};

struct sockaddr_at
{
  __SOCKADDR_COMMON (sat_);
  __uint8_t sat_port;
  struct atalk_addr sat_addr;
  __uint8_t sat_zero[8];
};

#endif	/* netatalk/at.h */
