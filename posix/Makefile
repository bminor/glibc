# Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
	   glob.h wordexp.h fnmatch.h gnu/types.h getopt.h		\
	   posix1_lim.h posix2_lim.h posix_opt.h local_lim.h tar.h	\
	   utsnamelen.h confname.h waitflags.h waitstatus.h sys/unistd.h

distribute := confstr.h

routines :=								      \
	uname								      \
	__times								      \
	__wait __waitpid __wait3 __wait4				      \
	alarm sleep pause						      \
	__fork __vfork _exit	      					      \
	__execve execve fexecve execv execle execl execvp execlp	      \
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
tests		:= tstgetopt testfnm
others		:= getconf
install-bin	:= getconf
install-lib	:= libposix.a
gpl2lgpl := getopt.c getopt1.c getopt.h	# Frob these guys' copying notices.

include ../Rules

$(objpfx)libposix.a: $(dep-dummy-lib); $(make-dummy-lib)
lib: $(objpfx)libposix.a

# Make the standalone glob/fnmatch package.

glob.tar: glob/ChangeLog glob/COPYING.LIB \
	  glob/Makefile.in glob/configure glob/configure.in glob/configure.bat\
	  glob/fnmatch.h glob/glob.h glob/fnmatch.c glob/glob.c	  
	tar cho$(verbose)f $@ $^
glob/%.c: %.c
	rm -f $@
	ln -s ../$< $@
glob/%.h: %.h
	rm -f $@
	ln -s ../$< $@

glob/configure: glob/configure.in
	cd glob; autoconf $(ACFLAGS)

glob/ChangeLog: ../ChangeLog
	changelog-extract --regexp 'posix/(glob|fnmatch).*' < $< > $@.new
	chmod a-w $@.new
	mv -f $@.new $@

%.Z: %
	compress -c $< > $@-tmp
	mv $@-tmp $@
%.z: %
	gzip -9 -c $< > $@-tmp
	mv $@-tmp $@
