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

ifndef hurd-srcdir-computed
hurd-srcdir-computed := yes

# If we were configured with `--with-hurd=DIR', then config.make sets
# hurd-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef hurd-srcdir
hurd-srcdir = ../hurd
endif

# hurd-srcdir is now set to the logical directory name.  This name might be
# relative to $(objdir), might be relative to the source directory $(..), or
# might be absolute.  We choose among these possibilities by looking for a
# canonical file in each of those places (in that order).
f := hurd/hurd_types.defs # Random file that identifies the Hurd source dir.
f := $(strip $f)
hurd-srcdir := $(firstword $(patsubst %/$f,%,$(wildcard $(addsuffix /$f,\
	       $(objpfx)$(hurd-srcdir) $(..)$(hurd-srcdir)))) $(hurd-srcdir))

endif	# ! hurd-srcdir-computed

ifdef in-Makerules

# For the time being, elide directories where nothing at all works.
subdirs := $(filter-out socket inet termios,$(subdirs)) # XXX

# Look for header files in hurd/ under the top-level library source directory.
includes += -I$(..)hurd

# When compiling, use the Hurd header files directly from the sources.
includes += -I$(hurd-srcdir)

# Marks temporary work-arounds installed by mib.
defines += -DMIB_HACKS
mib_hacks=t

# Find the Hurd header files in the Hurd source.
vpath hurd/%.h $(hurd-srcdir)

# Do not use any assembly code from sysdeps/unix (and subdirectories).
# This bypasses all the system call stubs and uses any existing posix or
# generic C files instead.
inhibit-sysdep-asm += unix*


ifneq (hurd,$(subdir))
ifneq (mach,$(subdir))
# Subdirectories other than hurd/ might use the generated Hurd headers.
# So make sure we get a chance to run in hurd/ to make them before all else.
# (But we don't want to do this in mach/, because hurd/ needs some things
# there, and we know mach/ doesn't need anything from hurd/.)

# These are all the generated headers that <hurd.h> includes.
before-compile += $(patsubst %,$(common-objpfx)hurd/%.h,io fs process)
$(patsubst %,$(common-objpfx)hurd/%.%,io fs process):
	$(MAKE) -C $(..)hurd generated no_deps=t
endif
endif

# Generate errnos.h and sys_errlist.c from the section of the manual that
# lists all the errno codes.

errno.texinfo = $(..)manual/errno.texi

hurd = $(..)sysdeps/mach/hurd

$(hurd)/errnos.h: $(hurd)/errnos.awk $(errno.texinfo)
	gawk -f $^ > $@-tmp
	mv $@-tmp $@
$(hurd)/errlist.c: $(hurd)/errlist.awk $(errno.texinfo)
	gawk -f $^ > $@-tmp
	mv $@-tmp $@

endif	# in-Makerules
