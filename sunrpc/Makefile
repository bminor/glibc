# Copyright (C) 1994 Free Software Foundation, Inc.
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
#	Sub-makefile for sunrpc portion of the library.
#
subdir	:= sunrpc

# The code in this subdirectory is taken verbatim from Sun's RPCSRC-4.0
# distribution.  A few files needed trivial modifications to compile in the
# GNU environment; these changes are marked by comments that say
# `roland@gnu'.  Other than that, several files were renamed so as not to
# exceed 14-character file name limits:
#
#	authunix_prot.c -> authuxprot.c
#	bindresvport.c -> bindrsvprt.c
#	clnt_generic.c -> clnt_gen.c
#	clnt_perror.c -> clnt_perr.c
#	clnt_simple.c -> clnt_simp.c
#	get_myaddress.c -> get_myaddr.c
#	pmap_getmaps.c -> pm_getmaps.c
#	pmap_getport.c -> pm_getport.c
#	rpc_callmsg.c -> rpc_cmsg.c
#	rpc_commondata.c -> rpc_common.c
#	rpc_dtablesize.c -> rpc_dtable.c
#	svc_auth_unix.c -> svc_authux.c
#	xdr_reference.c -> xdr_ref.c


headers := $(addprefix rpc/,auth.h auth_unix.h clnt.h netdb.h pmap_clnt.h \
		            pmap_prot.h pmap_rmt.h rpc.h rpc_msg.h svc.h \
			    svc_auth.h types.h xdr.h)

routines := auth_none auth_unix authuxprot bindrsvprt \
	    clnt_gen clnt_perr clnt_raw clnt_simp clnt_tcp \
	    clnt_udp rpc_dtable get_myaddr getrpcent getrpcport \
	    pmap_clnt pm_getmaps pm_getport pmap_prot \
	    pmap_prot2 pmap_rmt rpc_prot rpc_common rpc_cmsg \
	    svc svc_auth svc_authux svc_raw svc_run svc_simple \
	    svc_tcp svc_udp xdr xdr_array xdr_float xdr_mem \
	    xdr_rec xdr_ref xdr_stdio

# Sun's code is not too clean.
override +gccwarn :=# -w

include ../Rules
