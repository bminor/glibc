# Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for inet portion of the library.
#
subdir	:= inet

headers	:= netdb.h resolv.h \
	   netinet/in.h  \
	   sys/bitypes.h \
	   $(wildcard arpa/*.h protocols/*.h)
distribute := ../conf/portability.h

routines := ntohl ntohs htonl htons		\
	    inet_addr inet_lnaof inet_mkadr	\
	    inet_netof inet_ntoa inet_net	\
	    getnetbyad getnetbynm getnetent	\
	    getproto getprtent getprtname	\
	    getsrvbynm getsrvbypt getservent	\
	    res_comp res_debug res_init res_mkqry res_query res_send	\
	    gethstnmad sethostent	    \
	    rcmd rexec
aux	 := herror

# No warnings about losing BSD code.
override +gccwarn := -w

include ../Rules
