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
#	Sub-makefile for misc portion of the library.
#

subdir	:= misc

headers	:= sys/uio.h sys/ioctl.h sys/ptrace.h ioctls.h sys/file.h	\
	   a.out.h nlist.h stab.h stab.def sgtty.h sys/dir.h sys/cdefs.h \
	   ttyent.h

routines := brk sbrk sstk ioctl \
	    __brk __sbrk __ioctl \
	    readv writev \
	    setreuid setregid \
	    __setreuid __setregid \
	    seteuid setegid \
	    getpgsz \
	    __getpgsz \
	    getdtsz \
	    __getdtsz \
	    __gethstnm gethstnm sethostnam \
	    __select select \
	    acct chroot fsync sync reboot \
	    getdents \
	    __getdents \
	    gethostid sethostid \
	    mknod __mknod \
	    swapon vhangup mktemp \
	    ualarm usleep \
	    gtty stty \
	    ptrace \
	    nlist \
	    utimes \
	    __utimes \
	    truncate ftruncate \
	    chflags fchflags \
	    insremque swab getttyent
aux := data_start progname
distribute := bsd-compat.c
install-lib := libbsd-compat.a

include ../Rules

$(objpfx)libbsd-compat.a: $(objpfx)bsd-compat.o
	ln $< $@
