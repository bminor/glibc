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
# `roland@gnu'.  All the code from Sun's rpc, etc, and rpcgen
# subdirectories is in this directory; the rpc subdirectory contains only
# the header files.  Other than that, several files were renamed so as not
# to exceed 14-character file name limits:
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
#	rpcsvc/bootparam_prot.x -> rpcsvc/bootparam.x

headers = $(addprefix rpc/,auth.h auth_unix.h clnt.h netdb.h pmap_clnt.h \
			   pmap_prot.h pmap_rmt.h rpc.h rpc_msg.h svc.h \
			   svc_auth.h types.h xdr.h) \
	  $(rpcsvc:%=rpcsvc/%)
rpcsvc = bootparam.x nlm_prot.x rstat.x \
	 yppasswd.x klm_prot.x rex.x sm_inter.x mount.x \
	 rnusers.x spray.x nfs_prot.x rquota.x yp.x
# These headers are installed with rpcgen by a rule below.
install-others = $(rpcsvc:%.x=$(includedir)/rpcsvc/%.h) \
		 $(includedir)/rpcsvc/bootparam_prot.h \
		 $(etcdir)/rpc

routines := auth_none auth_unix authuxprot bindrsvprt \
	    clnt_gen clnt_perr clnt_raw clnt_simp clnt_tcp \
	    clnt_udp rpc_dtable get_myaddr getrpcent getrpcport \
	    pmap_clnt pm_getmaps pm_getport pmap_prot \
	    pmap_prot2 pmap_rmt rpc_prot rpc_common rpc_cmsg \
	    svc svc_auth svc_authux svc_raw svc_run svc_simple \
	    svc_tcp svc_udp xdr xdr_array xdr_float xdr_mem \
	    xdr_rec xdr_ref xdr_stdio

install-lib := librpcsvc.a
install := rpcgen rpcinfo portmap
rpcsvc-objs = $(rpcsvc:%.x=x%.o)
rpcgen-objs = rpc_main.o rpc_hout.o rpc_cout.o rpc_parse.o \
	      rpc_scan.o rpc_util.o rpc_svcout.o rpc_clntout.o
extra-objs = $(rpcgen-objs) $(rpcsvc-objs)
# These headers are part of rpcgen.
distribute := rpc_util.h rpc_parse.h rpc_scan.h $(rpcgen-objs:.o=.c) etc.rpc

others := portmap rpcinfo

# Sun's code is not too clean.
override +gccwarn :=

include ../Rules

$(objpfx)rpcgen: $(addprefix $(objpfx),$(rpcgen-objs)) $(libc.a)
	$(+link)

rpcgen-cmd = $(dir $(word 2,$^))$(notdir $(word 2,$^))

# Generate the installed rpcsvc headers in place with rpcgen.
$(includedir)/rpcsvc/%.h: rpcsvc/%.x $(objpfx)rpcgen
	$(rpcgen-cmd) -h $< -o $@

# The proper file name is longer than 14 chars, so we install it under
# a shorter name.  But if the filesystem can handle it, we want to
# install under the proper name as well.
$(includedir)/rpcsvc/bootparam_prot.h: $(includedir)/rpcsvc/bootparam.h
	@echo It is safe to ignore an error here if this file name is too long.
	-$(do-install)

# Install the rpc data base file.
$(etcdir)/rpc: etc.rpc
	$(do-install)

defines := $(defines) -D_PATH_RPC='"$(etcdir)/rpc"'

# Build the `rpcsvc' library of XDR functions.

$(objpfx)librpcsvc.a: $(rpcsvc:%.x=$(objpfx)x%.o)
# This library is small enough that it's simplest to recreate the archive
# from scratch each time.
	rm -f $@
ifdef objdir
	cd $(objdir); $(AR) cq$(verbose) $@ $(^:$(objpfx)%=%)
else
	$(AR) cq$(verbose) $@ $^
endif
	$(RANLIB) $@

$(objpfx)x%.o: rpcsvc/%.x $(objpfx)rpcgen
	$(rpcgen-cmd) -c $< -o $@
