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
#	Specific makefile for stdio.
#
subdir	:= stdio

headers	:= stdio.h stdio_lim.h printf.h

routines	:=							      \
	ctermid cuserid							      \
	feof ferror clearerr fileno					      \
	newstream fopen freopen fdopen fopncook fmemopen		      \
	setbuf setvbuf setbuffer setlinebuf				      \
	fseek ftell rewind fgetpos fsetpos				      \
	vfprintf vprintf printf_fp reg-printf printf-prs		      \
	vsnprintf vsprintf vasprintf	      				      \
	fprintf printf snprintf sprintf asprintf			      \
	dprintf vdprintf						      \
	__vfscanf vfscanf vscanf __vsscanf vsscanf			      \
	fscanf scanf sscanf						      \
	fread fwrite							      \
	ungetc								      \
	fgetc getc getchar getw						      \
	fputc putc putchar putw						      \
	fgets gets fputs puts						      \
	getdelim getline __getdelim __getline				      \
	perror psignal							      \
	tmpfile tmpnam tempnam						      \
	fclose fflush							      \
	remove rename							      \
	memstream							      \
	internals sysd-stdio pipestream stdio_init
routines := $(strip $(routines))
aux	:= errlist siglist defs syms-stdio glue

tests := bug1 bug2 bug3 bug4 bug5 tst-printf tstscanf test_rdwr test-popen \
	 tstgetline


include ../Rules
