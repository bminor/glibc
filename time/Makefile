# Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#	Makefile for time routines
#
subdir	:= time

headers	:= time.h sys/time.h gnu/time.h
distribute := tzfile.h private.h scheck.c ialloc.c emkdir.c

routines	:= offtime asctime clock ctime difftime gmtime	\
		   localtime mktime strftime time __tzset tzfile \
		   __gettod settod adjtime getitmr \
		   __getitmr __adjtime __settod \
		   __setitmr tzset gettod setitmr
aux		:= syms-time

others	:= ap zdump zic
tests	:= test_time

tzfiles := africa antarctica asia australasia europe northamerica \
	   southamerica pacificnew etcetera factory systemv
distribute := $(distribute) $(tzfiles) leapseconds

install := zic zdump

define \n


endef
include zonenames
zonenames := $(subst $(\n), ,$(zonenames))

include ../Makeconfig
ifdef localtime
zonenames := $(zonenames) localtime
endif

install-data := $(addprefix zoneinfo/,$(zonenames))


include ../Rules


$(tzfiles:%=zones-%): zones-%: %
	(echo 'define $*-zones'						;\
	 awk '$$1 == "Zone" { print $$2 } $$1 == "Link" { print $$3 }' $^;\
	 echo 'endef'							;\
	 echo '$*-zones := $$(subst $$(\\n), ,$$($*-zones))'		;\
	 echo 'ifdef $*-zones'						;\
	 echo '$$(addprefix %/,$$($*-zones)): \'			;\
	 echo '$< $$(objpfx)zic leapseconds zoneinfo/'			;\
	 echo '	$$(tzcompile)'						;\
	 echo 'endif'							;\
	) > $@
zonenames: Makefile
	(for file in $(tzfiles); do	\
	   echo "include zones-$$file";	\
	 done) > $@

# Make all the zoneinfo files.
.PHONY: zones zoneinfo
zones: zoneinfo
zoneinfo: $(foreach file,$(tzfiles),$(addprefix zoneinfo/,$($(file)-zones)))

define tzcompile
$(word 2,$^) -d zoneinfo -L $(word 3,$^) $<
endef

ifdef localtime
zoneinfo/localtime: zoneinfo/$(localtime) $(objpfx)zic
	$(word 2,$^) -d zoneinfo -l $(<:zoneinfo/%=%)
zoneinfo: zoneinfo/localtime
endif

$(objpfx)zic: $(objpfx)scheck.o $(objpfx)ialloc.o $(objpfx)emkdir.o

$(objpfx)tzfile.o: tzfile.c
	$(COMPILE.c) -DTZDIR='"$(datadir)/zoneinfo"' $< $(OUTPUT_OPTION)
