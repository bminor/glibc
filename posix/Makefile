# Copyright (C) 1991 Free Software Foundation, Inc.
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

headers	:= sys/utsname.h sys/times.h sys/wait.h sys/types.h sys/stat.h	\
	   utime.h fcntl.h unistd.h glob.h wordexp.h posix1_limits.h	\
	   posix2_limits.h posix_options.h confstr.h fnmatch.h		\
	   local_limits.h						\
	   gnu/stat.h gnu/wait.h gnu/types.h gnu/fcntl.h

routines :=								      \
	utime								      \
	mkfifo								      \
	uname								      \
	__times								      \
	__wait __waitpid __wait3 __wait4				      \
	__stat __fstat __lstat						      \
	__umask __chmod __fchmod __mkdir				      \
	__open __close __read __write __lseek __access __fcntl flock	      \
	__dup __dup2 __pipe						      \
	creat								      \
	__chdir getcwd getwd						      \
	alarm sleep pause						      \
	__chown __fchown						      \
	__fork __vfork _exit	      					      \
	__execve execve execv execle execl execvp execlp		      \
	__getpid __getppid						      \
	__getuid __geteuid __getgid __getegid __getgroups __setuid __setgid   \
	ttyname __isatty						      \
	__getpgrp __setpgrp __setsid setpgid				      \
	__link __symlink __readlink					      \
	__unlink __rmdir						      \
	getlogin setlogin						      \
	__pathconf __sysconf __fpathconf				      \
	glob fnmatch							      \
	confstr								      \
	getopt
routines	:= $(sort $(foreach f,$(routines),$f $(f:__%=%)))
aux		:= init-posix environ
tests := test-getopt

others		:= pwd logname getconf id

include ../Rules

# Make the standalone glob/fnmatch package.
glob.tar.Z: glob.tar
	compress -c $< > $@
glob.tar: glob/COPYING.LIB glob/Makefile \
	  glob/fnmatch.h glob/glob.h glob/fnmatch.c glob/glob.c	  
	tar ch$(verbose)f $@ $^
glob/%.c: %.c
	$(..)ansidecl -trad $< | indent -stdin -gnu > $@
glob/%.h: %.h
	$(..)ansidecl -trad $< | indent -stdin -gnu > $@
