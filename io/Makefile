# Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
#	Sub-makefile for I/O portion of the library.
#
subdir	:= io

headers := sys/stat.h statbuf.h \
	   fcntl.h sys/fcntl.h fcntlbits.h \
	   poll.h sys/poll.h \
	   utime.h ftw.h

routines :=								      \
	utime								      \
	mkfifo								      \
	__stat __fstat __lstat						      \
	__umask __chmod __fchmod __mkdir				      \
	__open __close __read __write __lseek __access			      \
	__fcntl __flock lockf						      \
	__dup __dup2 __pipe						      \
	creat								      \
	__chdir fchdir							      \
	getcwd getwd getdirname						      \
	__chown __fchown						      \
	ttyname __isatty						      \
	__link __symlink __readlink					      \
	__unlink __rmdir						      \
	ftw poll
routines	:= $(sort $(foreach f,$(routines),$f $(f:__%=%)))

others		:= pwd
tests		:= test-utime

include ../Rules
