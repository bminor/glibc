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
#	Sub-makefile for POSIX portion of the library.
#
subdir	:= posix

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h unistd.h	\
	   glob.h wordexp.h fnmatch.h gnu/wait.h gnu/types.h getopt.h	\
	   posix1_lim.h posix2_lim.h posix_opt.h local_lim.h tar.h

distribute := confstr.h utsnamelen.h

routines :=								      \
	uname								      \
	__times								      \
	__wait __waitpid __wait3 __wait4				      \
	alarm sleep pause						      \
	__fork __vfork _exit	      					      \
	__execve execve execv execle execl execvp execlp		      \
	__getpid __getppid						      \
	__getuid __geteuid __getgid __getegid __getgrps __setuid __setgid     \
	__getpgrp __setpgrp __setsid setpgid				      \
	getlogin setlogin						      \
	__pathconf __sysconf __fpathcon					      \
	glob fnmatch							      \
	confstr								      \
	getopt getopt1
routines	:= $(sort $(foreach f,$(routines),$f $(f:__%=%)))
aux		:= init-posix environ
tests		:= tstgetopt
others		:= getconf
install-lib	:= libposix.a

include ../Rules

$(objpfx)libposix.a:
	$(AR) cr $@ /dev/null

# Make the standalone glob/fnmatch package.
glob.tar.Z: glob.tar
	compress -c $< > $@-tmp
	mv $@-tmp $@
glob.tar: glob/COPYING.LIB glob/Makefile \
	  glob/fnmatch.h glob/glob.h glob/fnmatch.c glob/glob.c	  
	tar ch$(verbose)f $@ $^
glob/%.c: %.c
	$(..)ansidecl -trad $< | indent -stdin -gnu > $@-tmp
	mv $@-tmp $@
glob/%.h: %.h
	$(..)ansidecl -trad $< | indent -stdin -gnu > $@-tmp
	mv $@-tmp $@
