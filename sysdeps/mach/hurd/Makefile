# Copyright (C) 1993 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.


# If we were configured with `--with-hurd=DIR', then config.make sets
# hurd-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef hurd-srcdir
hurd-srcdir = ../../hurd
endif

# Look for header files in hurd/ under the top-level library source directory.
includes += -I$(..)hurd

# When compiling, use the Hurd header files directly from the sources.
includes += -I$(hurd-srcdir)

# Find the Hurd header files in the Hurd source.
vpath hurd/%.h $(hurd-srcdir)

# Generate errnos.h and sys_errlist.c from the section of the manual that
# lists all the errno codes.

errno.texinfo = $(..)manual/errno.texi

hurd = $(..)sysdeps/mach/hurd

$(hurd)/errnos.h: $(hurd)/errnos.awk $(errno.texinfo)
	awk -f $^ > $@-tmp
	mv $@-tmp $@
$(hurd)/errlist.c: $(hurd)/errlist.awk $(errno.texinfo)
	awk -f $^ > $@-tmp
	mv $@-tmp $@
