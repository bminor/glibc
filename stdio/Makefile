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
	tmpfile tmpnam tempnam tempname					      \
	fclose fflush							      \
	remove rename							      \
	memstream obstream						      \
	internals sysd-stdio pipestream stdio_init libc_fatal

# Several mpn functions from GNU MP are used by the printf_fp function.
mpn-routines := add_1 add_n addmul_1 cmp divmod divmod_1 \
		lshift rshift mod_1 mul mul_1 mul_n sub_n submul_1 
mpn-headers = longlong.h gmp.h gmp-impl.h gmp-mparam.h asm.h

routines := $(strip $(routines) $(mpn-routines))
aux	:= errlist siglist defs syms-stdio glue
distribute = $(mpn-headers) gmp2glibc.sed

tests := tst-printf tstscanf test_rdwr test-popen tstgetln test-fseek \
	 bug1 bug2 bug3 bug4 bug5 bug6 temptest


include ../Rules


ifdef gmp-srcdir

gmp-srcdir := $(firstword $(filter-out ..//%,$(..)$(gmp-srcdir) $(gmp-srcdir)))

# Copy the mpn source files we use from the GNU MP source directory.
# `gmp-srcdir' is set by doing `configure --with-gmp=DIR'.
# (Do not try this at home.  You need an as yet unreleased version of GNU MP.)

mpn-sysdep := $(addsuffix .c,$(mpn-routines)) \
	      $(addsuffix .S,$(mpn-routines)) gmp-mparam.h asm.h

mpn-try := $(addprefix $(gmp-srcdir)/mpn/*/,$(mpn-sysdep))
mpn-found := $(wildcard $(mpn-try))

# We call `m68k' the directory GMP calls `mc68020'.

mpn-found-68k := $(filter $(gmp-srcdir)/mpn/mc68020/%,$(mpn-found))
mpn-found-no-68k := $(filter-out $(gmp-srcdir)/mpn/mc68020/%,$(mpn-found))

mpn-no-m68k := $(patsubst $(gmp-srcdir)/mpn/%,$(sysdep_dir)/%,\
			  $(mpn-found-no-68k))
$(mpn-no-m68k): $(sysdep_dir)/%: gmp2glibc.sed $(gmp-srcdir)/mpn/%
	$(gmp2glibc)

mpn-m68k := $(patsubst $(gmp-srcdir)/mpn/mc68020/%,$(sysdep_dir)/m68k/%,\
		       $(mpn-found-68k))
$(mpn-m68k): $(sysdep_dir)/m68k/%: gmp2glibc.sed $(gmp-srcdir)/mpn/mc68020/%
	$(gmp2glibc)

mpn-stuff = $(mpn-m68k) $(mpn-no-m68k) $(mpn-copy)

# chmod so I don't edit them by mistake.
define gmp2glibc
sed -f $^ > $@-tmp
chmod a-w $@-tmp
mv -f $@-tmp $@
endef

mpn-copy = $(filter-out $(mpn-sysdep),$(mpn-headers)) # mp_bases.c
$(mpn-copy): %: gmp2glibc.sed $(gmp-srcdir)/%; $(gmp2glibc)

.PHONY: copy-mpn
copy-mpn: $(mpn-stuff)

endif
