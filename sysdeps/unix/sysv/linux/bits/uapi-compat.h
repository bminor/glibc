/* Copyright (C) 2019 Free Software Foundation, Inc.
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

/* This header is internal to glibc and should not be included outside
   of glibc headers.  It is included by each header that needs to make
   global declarations that may or may not already have been made by a
   header provided by the underlying operating system kernel.  All such
   declarations are associated with macros named __UAPI_DEF_something,
   which have three possible values:

   If __UAPI_DEF_FOO is not defined, nobody has yet declared `foo'.
   If __UAPI_DEF_FOO is defined to 0, glibc's headers have declared `foo'.
   If __UAPI_DEF_FOO is defined to 1, the kernel's headers have declared `foo`.

   This header cannot have a multiple-inclusion guard, because it needs
   to recheck for additional declarations by kernel headers each time
   a glibc header that uses it is included.

   This version of uapi-compat.h is used for Linux.  Sufficiently new
   versions of the Linux kernel headers will define the __UAPI_DEF_*
   macros themselves; this header's job is to preserve compatibility
   with older versions of the headers that don't do this.

   The guard macros on many of Linux's UAPI headers were changed in
   kernel rev 56c176c9; we need to check them both with and without a
   _UAPI_ prefix.  */

#if defined _UAPI_LINUX_IF_H || defined _LINUX_IF_H

#ifndef __UAPI_DEF_IF_IFCONF
#define __UAPI_DEF_IF_IFCONF 1
#endif
#ifndef __UAPI_DEF_IF_IFMAP
#define __UAPI_DEF_IF_IFMAP 1
#endif
#ifndef __UAPI_DEF_IF_IFNAMSIZ
#define __UAPI_DEF_IF_IFNAMSIZ 1
#endif
#ifndef __UAPI_DEF_IF_IFREQ
#define __UAPI_DEF_IF_IFREQ 1
#endif
#ifndef __UAPI_DEF_IF_NET_DEVICE_FLAGS
#define __UAPI_DEF_IF_NET_DEVICE_FLAGS 1
#endif
#ifndef __UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO
#define __UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO 1
#endif

#endif /* linux/if.h */

#if defined _UAPI_LINUX_IN_H || defined _LINUX_IN_H

#ifndef __UAPI_DEF_IN_ADDR
#define __UAPI_DEF_IN_ADDR		1
#endif
#ifndef __UAPI_DEF_IN_IPPROTO
#define __UAPI_DEF_IN_IPPROTO		1
#endif
#ifndef __UAPI_DEF_IN_PKTINFO
#define __UAPI_DEF_IN_PKTINFO		1
#endif
#ifndef __UAPI_DEF_IP_MREQ
#define __UAPI_DEF_IP_MREQ		1
#endif
#ifndef __UAPI_DEF_SOCKADDR_IN
#define __UAPI_DEF_SOCKADDR_IN		1
#endif
#ifndef __UAPI_DEF_IN_CLASS
#define __UAPI_DEF_IN_CLASS		1
#endif

#endif /* linux/in.h  */

#if defined _UAPI_LINUX_IN6_H || defined _LINUX_IN6_H

#ifndef __UAPI_DEF_IN6_ADDR
#define __UAPI_DEF_IN6_ADDR 1
#endif
#ifndef __UAPI_DEF_IN6_ADDR_ALT
# if defined __USE_MISC || defined __USE_GNU
#  define __UAPI_DEF_IN6_ADDR_ALT 1
# else
#  define __UAPI_DEF_IN6_ADDR_ALT 0
# endif
#endif
#ifndef __UAPI_DEF_SOCKADDR_IN6
#define __UAPI_DEF_SOCKADDR_IN6 1
#endif
#ifndef __UAPI_DEF_IPV6_MREQ
#define __UAPI_DEF_IPV6_MREQ 1
#endif
#ifndef __UAPI_DEF_IPPROTO_V6
#define __UAPI_DEF_IPPROTO_V6 1
#endif
#ifndef __UAPI_DEF_IPV6_OPTIONS
#define __UAPI_DEF_IPV6_OPTIONS 1
#endif

#endif /* linux/in6.h */

#if defined _UAPI_IPV6_H || defined _IPV6_H

#ifndef __UAPI_DEF_IN6_PKTINFO
#define __UAPI_DEF_IN6_PKTINFO 1
#endif
#ifndef __UAPI_DEF_IP6_MTUINFO
#define __UAPI_DEF_IP6_MTUINFO 1
#endif

#endif /* linux/ipv6.h */

#if defined _UAPI_IPX_H_ || defined _IPX_H_

#ifndef __UAPI_DEF_SOCKADDR_IPX
#define __UAPI_DEF_SOCKADDR_IPX			1
#endif
#ifndef __UAPI_DEF_IPX_ROUTE_DEFINITION
#define __UAPI_DEF_IPX_ROUTE_DEFINITION		1
#endif
#ifndef __UAPI_DEF_IPX_INTERFACE_DEFINITION
#define __UAPI_DEF_IPX_INTERFACE_DEFINITION	1
#endif
#ifndef __UAPI_DEF_IPX_CONFIG_DATA
#define __UAPI_DEF_IPX_CONFIG_DATA		1
#endif
#ifndef __UAPI_DEF_IPX_ROUTE_DEF
#define __UAPI_DEF_IPX_ROUTE_DEF		1
#endif

#endif /* linux/ipx.h */

#if defined _UAPI_LINUX_XATTR_H || defined _LINUX_XATTR_H \
 || defined __USE_KERNEL_XATTR_DEFS

#ifndef __UAPI_DEF_XATTR
#define __UAPI_DEF_XATTR		1
#endif

#endif /* linux/xattr.h */
