/* Checking macros for socket functions.
   Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_SOCKET_H
# error "Never include <bits/socket2.h> directly; use <sys/socket.h> instead."
#endif

extern void __chk_fail (void) __attribute__((noreturn));
#define recv(fd, buf, n, flags) \
  (__extension__							      \
    ({ size_t __n_val = (n);				  		      \
       if (__bos0 (buf) != (size_t) -1 && __bos0 (buf) < __n_val)	      \
         __chk_fail ();							      \
       recv ((fd), (buf), __n_val, (flags)); }))

#define recvfrom(fd, buf, n, flags, addr, addr_len) \
  (__extension__							      \
    ({ size_t __n_val = (n);				  		      \
       if (__bos0 (buf) != (size_t) -1 && __bos0 (buf) < __n_val)	      \
         __chk_fail ();							      \
       recvfrom ((fd, (buf), __n_val, (flags), (addr), (addr_len)); }))
