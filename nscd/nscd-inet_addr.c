/* Legacy IPv4 text-to-address functions.  Version for nscd.
   Copyright (C) 2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <arpa/inet.h>

/* We do not want to export __inet_aton_exact.  Get the prototype and
   change the visibility to hidden.  */
#include <arpa/inet.h>
__typeof__ (__inet_aton_exact) __inet_aton_exact
  __attribute__ ((visibility ("hidden")));

/* Do not provide definitions of the public symbols exported from
   libc.  */
#undef weak_alias
#define weak_alias(from, to)

#include <resolv/inet_addr.c>
