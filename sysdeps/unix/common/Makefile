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

$(common-objpfx)local_lim.h: $(common-objpfx)make-local_lim
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@
$(common-objpfx)make-local_lim: $(sysdep_dir)/unix/bsd/make-local_lim.c
	$(native-compile)

before-compile := $(before-compile) $(common-objpfx)local_lim.h

ifeq ($(subdir),posix)
sysdep_routines := $(sysdep_routines) bsd_getgrp
endif

ifeq ($(subdir),ctype)

sysdep_routines := $(sysdep_routines) ctype-glue

$(objpfx)ctype-glue.c: $(objpfx)glue-ctype
	@rm -f $@
	$< > $@-tmp
	mv $@-tmp $@

$(objpfx)glue-ctype: $(sysdep_dir)/unix/bsd/glue-ctype.c
	$(native-compile)

endif

ifeq ($(subdir),stdio)

before-compile := $(before-compile) $(objpfx)siglist.c
$(objpfx)siglist.c: $(objpfx)make_siglist
	@rm -f $@
	$< > $@-tmp
	mv $@-tmp $@

$(objpfx)make_siglist: $(sysdep_dir)/unix/bsd/make_siglist.c
	$(native-compile)

endif
