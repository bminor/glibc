# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.

# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

# $Id$

subdir	:= glibc-compat

distribute		:= nss-nis.h

# This is the trivial part which goes into libc itself.
routines		=

# These are the databases that go through nss dispatch.
# Caution: if you add a database here, you must add its real name
# in databases.def, too.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp alias

# Specify rules for the nss_* modules.  We have some services.
services		:= files nis compat dns

extra-libs		:= $(services:%=libnss1_%) libNoVersion
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)

libnss1_files-routines	:= $(addprefix files-,$(databases))
libnss1_compat-routines	:= $(addprefix compat-,grp pwd spwd)
libnss1_nis-routines	:= $(addprefix nis-,$(databases))
libnss1_dns-routines	:= $(addprefix dns-, host network)

libcompat-routines	:= $(addprefix old, fileops iofdopen iopopen stdfiles \
					    iofclose  iofopen pclose tmpfile)
libNoVersion-routines	:= stubs

libnss1_files-inhibit-o	= $(filter-out .os,$(object-suffixes))
libnss1_compat-inhibit-o = $(filter-out .os,$(object-suffixes))
libnss1_nis-inhibit-o	= $(filter-out .os,$(object-suffixes))
libnss1_dns-inhibit-o	= $(filter-out .os,$(object-suffixes))

-include ../Rules

# Force the soname to be libnss_*.so.1 for compatibility.
LDFLAGS-nss1_files.so	= -Wl,-soname=lib$(libprefix)nss_files.so$($(@F)-version)
LDFLAGS-nss1_nis.so	= -Wl,-soname=lib$(libprefix)nss_nis.so$($(@F)-version)
LDFLAGS-nss1_compat.so	= -Wl,-soname=lib$(libprefix)nss_compat.so$($(@F)-version)
LDFLAGS-nss1_dns.so	= -Wl,-soname=lib$(libprefix)nss_dns.so$($(@F)-version)

-include ../Makeconfig

ifeq (yes,$(build-shared))
install-others	+= $(inst_slibdir)/libnss_files.so$(libnss1_files.so-version) \
		   $(inst_slibdir)/libnss_nis.so$(libnss1_nis.so-version) \
		   $(inst_slibdir)/libnss_compat.so$(libnss1_compat.so-version) \
		   $(inst_slibdir)/libnss_dns.so$(libnss1_dns.so-version)
endif

$(inst_slibdir)/libnss_files.so$(libnss1_files.so-version): $(inst_slibdir)/libnss1_files-$(version).so $(+force)
	rm -f $@
	$(LN_S) $(<F) $@

$(inst_slibdir)/libnss_nis.so$(libnss1_nis.so-version): $(inst_slibdir)/libnss1_nis-$(version).so $(+force)
	rm -f $@
	$(LN_S) $(<F) $@

$(inst_slibdir)/libnss_compat.so$(libnss1_compat.so-version): $(inst_slibdir)/libnss1_compat-$(version).so $(+force)
	rm -f $@
	$(LN_S) $(<F) $@

$(inst_slibdir)/libnss_dns.so$(libnss1_dns.so-version): $(inst_slibdir)/libnss1_dns-$(version).so $(+force)
	rm -f $@
	$(LN_S) $(<F) $@


$(objpfx)libnss1_compat.so: $(common-objpfx)nis/libnsl.so$(libnsl.so-version) \
			   $(objpfx)libnss1_files.so
$(objpfx)libnss1_nis.so: $(common-objpfx)nis/libnsl.so$(libnsl.so-version) \
			$(objpfx)libnss1_files.so

# The DNS NSS modules needs the resolver.
#$(objpfx)libnss1_dns.so: $(filter-out $(common-objpfx)resolv/stamp.os, \
#				$(wildcard $(common-objpfx)resolv/*.os)) \
#			$(common-objpfx)libc.so
$(objpfx)libnss1_dns.so: $(common-objpfx)resolv/libresolv.so $(common-objpfx)libc.so

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libnss1_compat.so: $(common-objpfx)nis/libnsl.so$(libnsl.so-version) \
                            $(objpfx)libnss1_files.so $(common-objpfx)libc.so
$(objpfx)libnss1_dns.so: $(common-objpfx)resolv/libresolv.so \
                         $(common-objpfx)libc.so
$(objpfs)libnss1_files.so: $(common-objpfx)libc.so
$(objpfx)libnss1_nis.so: $(common-objpfx)nis/libnsl.so$(libnsl.so-version) \
                         $(objpfx)libnss1_files.so $(common-objpfx)libc.so

check-abi-libNoVersion: $(..)scripts/extract-abilist.awk
	@:
update-abi-libNoVersion: $(..)scripts/merge-abilist.awk
	@:
check-abi-libnss1_compat: $(..)scripts/extract-abilist.awk
	@:
update-abi-libnss1_compat: $(..)scripts/merge-abilist.awk
	@:
check-abi-libnss1_dns: $(..)scripts/extract-abilist.awk
	@:
update-abi-libnss1_dns: $(..)scripts/merge-abilist.awk
	@:
check-abi-libnss1_files: $(..)scripts/extract-abilist.awk
	@:
update-abi-libnss1_files: $(..)scripts/merge-abilist.awk
	@:
check-abi-libnss1_nis: $(..)scripts/extract-abilist.awk
	@:
update-abi-libnss1_nis: $(..)scripts/merge-abilist.awk
	@:

#
# This is needed to build the separate tarball
#
pkgNAME		= $(subdir)
pkgVERSION	= 2.1.3
pkgCVSTAG	= $(pkgNAME)_$(subst .,-,$(pkgVERSION))

archive:
	@rm -f *.tar.gz *~
	cvs tag -F $(pkgCVSTAG) .
	@rm -rf /tmp/$(pkgNAME)-$(pkgVERSION) /tmp/$(pkgNAME) $(pkgNAME)-$(pkgVERSION).tar.gz
	@cd /tmp; cvs export -r$(pkgCVSTAG) $(pkgNAME)
	@pkgDIR=$$PWD; cd /tmp; tar cvzf $$pkgDIR/$(pkgNAME)-$(pkgVERSION).tar.gz $(pkgNAME)
	@rm -rf /tmp/$(pkgNAME)
	@echo "The archive is in $(pkgNAME)-$(pkgVERSION).tar.gz"
