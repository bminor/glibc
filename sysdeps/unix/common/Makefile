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

$(common-objpfx)local_lim.h: $(common-objpfx)mk-local_lim
	$(dir $<)$(notdir $<) > $@-t
	mv $@-t $@
$(common-objpfx)mk-local_lim: $(sysdep_dir)/unix/common/mk-local_lim.c
	$(native-compile)

before-compile := $(before-compile) $(common-objpfx)local_lim.h
generated := $(generated) \
	     $(common-objpfx)local_lim.h $(common-objpfx)mk-local_lim

ifeq ($(subdir),posix)
sysdep_routines := $(sysdep_routines) bsd_getgrp
endif

ifeq ($(subdir),ctype)

sysdep_routines := $(sysdep_routines) ctype-glue

generated := $(generated) $(objpfx)glue-ctype $(objpfx)glue-ctype.c

$(objpfx)ctype-glue.c: $(objpfx)glue-ctype
	@rm -f $@
	$(dir $<)$(notdir $<) > $@-t
	mv $@-t $@

$(objpfx)glue-ctype: $(sysdep_dir)/unix/common/glue-ctype.c
	$(native-compile)

endif

ifeq ($(subdir),stdio)

before-compile := $(before-compile) $(objpfx)siglist.c
$(objpfx)siglist.c: $(objpfx)make_siglist
	@rm -f $@
	$< > $@-tmp
	mv $@-tmp $@

$(objpfx)make_siglist: $(sysdep_dir)/unix/common/make_siglist.c
	$(native-compile)

generated := $(generated) $(objpfx)siglist.c $(objpfx)make_siglist

endif
