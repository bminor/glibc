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
	   southamerica etcetera factory systemv
# pacificnew doesn't compile; if it is to be used, it should be included in
# northamerica.
distribute := $(distribute) $(tzfiles) leapseconds pacificnew

install := zic zdump

define nl


endef
include zonenames
zonenames := $(subst $(nl), ,$(zonenames))

install-data = $(addprefix zoneinfo/,$(zonenames) \
			   	     $(filter-out /%,$(localtime-file) \
						     $(posixrules-file)))
install-others = $(filter /%,$(localtime-file) $(posixrules-file))

include ../Rules

$(tzfiles:%=zones-%): zones-%: %
	(echo 'define $*-zones'						;\
	 awk '$$1 == "Zone" { print $$2 } $$1 == "Link" { print $$3 }' $^;\
	 echo 'endef'							;\
	 echo '$*-zones := $$(subst $$(nl), ,$$($*-zones))'		;\
	 echo 'ifdef $*-zones'						;\
	 echo '$$(addprefix zoneinfo/%/,$$($*-zones)): \'		;\
	 echo '$< $$(objpfx)zic leapseconds zoneinfo/'			;\
	 echo '	$$(tzcompile)'						;\
	 echo 'endif'							;\
	 echo 'define zonenames'					;\
	 echo '$$($*-zones)'						;\
	 echo 'endef'							;\
	 echo 'zonenames := $$(subst $$(nl), ,$$(zonenames))'		;\
	) > $@
zonenames: Makefile
	(for file in $(tzfiles); do	\
	   echo "include zones-$$file";	\
	 done) > $@

.PHONY: echo-zonenames
echo-zonenames: zonenames
	@echo 'Known zones: $(zonenames)'

# Make all the zoneinfo files.
.PHONY: zones zoneinfo
zones: zoneinfo
zoneinfo: $(foreach file,$(tzfiles),$(addprefix zoneinfo/,$($(file)-zones)))

define tzcompile
$(word 2,$^) -d $(patsubst %/,%,$(word 4,$^)) -L $(word 3,$^) $<
endef

ifdef localtime
zoneinfo/localtime: zoneinfo/$(localtime) $(objpfx)zic
	$(word 2,$^) -d zoneinfo -l $(<:zoneinfo/%=%)
zoneinfo: zoneinfo/localtime
endif
ifdef posixrules
zoneinfo/posixrules: zoneinfo/$(posixrules) $(objpfx)zic
	$(word 2,$^) -d zoneinfo -p $(<:zoneinfo/%=%)
zoneinfo: zoneinfo/posixrules
endif
ifneq ($(filter /%,$(localtime-file)),)
$(localtime-file): zoneinfo/$(localtime); $(do-install)
endif
ifneq ($(filter /%,$(posixrules-file)),)
$(posixrules-file): zoneinfo/$(posixrules); $(do-install)
endif


$(objpfx)zic: $(objpfx)scheck.o $(objpfx)ialloc.o $(objpfx)emkdir.o

$(objpfx)tzfile.o: tzfile.c; $(tz-cc)
$(objpfx)zic.o: zic.c; $(tz-cc)
define tz-cc
$(COMPILE.c) $(+gcc-nowarn) \
	     -DTZDIR='"$(datadir)/zoneinfo"' \
	     -DTZDEFAULT='"$(localtime-file)"' \
	     -DTZDEFRULES='"$(posixrules-file)"' \
	     $< $(OUTPUT_OPTION)
endef
