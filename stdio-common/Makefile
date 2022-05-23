# Copyright (C) 1991-2022 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

#
#	Specific makefile for stdio-common.
#
subdir	:= stdio-common

include ../Makeconfig

headers	:= stdio_ext.h printf.h bits/printf-ldbl.h bits/stdio_lim.h

routines := \
  _itoa \
  _itowa \
  asprintf \
  ctermid \
  cuserid \
  dprintf \
  flockfile \
  fprintf \
  fscanf \
  ftrylockfile \
  funlockfile \
  gentempfd \
  getline \
  getw \
  iovfscanf \
  isoc99_fscanf \
  isoc99_scanf \
  isoc99_sscanf \
  isoc99_vfscanf \
  isoc99_vscanf \
  isoc99_vsscanf \
  itoa-digits \
  itoa-udigits \
  itowa-digits \
  perror \
  printf \
  printf-prs \
  printf_fp \
  printf_fphex \
  printf_size \
  psiginfo \
  psignal \
  putw \
  reg-modifier \
  reg-printf \
  reg-type \
  remove \
  rename \
  renameat \
  renameat2 \
  scanf \
  snprintf \
  sprintf \
  sscanf \
  tempnam \
  tempname \
  tmpfile \
  tmpfile64 \
  tmpnam \
  tmpnam_r \
  vfprintf \
  vfprintf-internal \
  vfscanf \
  vfscanf-internal \
  vfwprintf \
  vfwprintf-internal \
  vfwscanf \
  vfwscanf-internal \
  vprintf \
  # routines

aux := \
  errlist \
  errlist-data \
  errname \
  fxprintf \
  printf-parsemb \
  printf-parsewc \
  siglist \
  # aux

tests := \
  bug-vfprintf-nargs \
  bug1 \
  bug10 \
  bug11 \
  bug12 \
  bug13 \
  bug14 \
  bug16 \
  bug17 \
  bug18 \
  bug18a \
  bug19 \
  bug19a \
  bug2 \
  bug20 \
  bug21 \
  bug22 \
  bug23 \
  bug23-2 \
  bug23-3 \
  bug23-4 \
  bug24 \
  bug25 \
  bug26 \
  bug3 \
  bug4 \
  bug5 \
  bug6 \
  bug7 \
  bug8 \
  bug9 \
  errnobug \
  scanf1 \
  scanf10 \
  scanf11 \
  scanf12 \
  scanf13 \
  scanf14 \
  scanf14a \
  scanf15 \
  scanf16 \
  scanf16a \
  scanf17 \
  scanf2 \
  scanf3 \
  scanf4 \
  scanf5 \
  scanf7 \
  scanf8 \
  scanf9 \
  temptest \
  test-fseek \
  test-fwrite \
  test-popen \
  test-strerr \
  test-vfprintf \
  test_rdwr \
  tfformat \
  tiformat \
  tllformat \
  tst-bz11319 \
  tst-bz11319-fortify2 \
  tst-cookie \
  tst-fdopen \
  tst-ferror \
  tst-fgets \
  tst-fileno \
  tst-fmemopen \
  tst-fmemopen2 \
  tst-fmemopen3 \
  tst-fmemopen4 \
  tst-fphex \
  tst-fphex-wide \
  tst-fseek \
  tst-fwrite \
  tst-gets \
  tst-grouping \
  tst-long-dbl-fphex \
  tst-memstream-string \
  tst-obprintf \
  tst-perror \
  tst-popen \
  tst-popen2 \
  tst-printf-binary \
  tst-printf-bz18872 \
  tst-printf-bz25691 \
  tst-printf-fp-free \
  tst-printf-fp-leak \
  tst-printf-round \
  tst-printfsz \
  tst-put-error \
  tst-renameat2 \
  tst-rndseek \
  tst-scanf-round \
  tst-setvbuf1 \
  tst-sprintf \
  tst-sprintf-errno \
  tst-sprintf2 \
  tst-sprintf3 \
  tst-sscanf \
  tst-swprintf \
  tst-swscanf \
  tst-tmpnam \
  tst-ungetc \
  tst-unlockedio \
  tst-vfprintf-mbs-prec \
  tst-vfprintf-user-type \
  tst-vfprintf-width-prec \
  tst-vfprintf-width-prec-alloc \
  tst-vfprintf-width-i18n \
  tst-wc-printf \
  tstdiomisc \
  tstgetln \
  tstscanf \
  xbug \
  # tests

generated += \
  errlist-data-aux-shared.S \
  errlist-data-aux.S \
  siglist-aux-shared.S \
  siglist-aux.S \
  # generated

test-srcs = tst-unbputc tst-printf tst-printfsz-islongdouble

ifeq ($(run-built-tests),yes)
tests-special += \
  $(objpfx)tst-printf-bz18872-mem.out \
  $(objpfx)tst-printf-bz25691-mem.out \
  $(objpfx)tst-printf-fp-free-mem.out \
  $(objpfx)tst-printf-fp-leak-mem.out \
  $(objpfx)tst-printf.out \
  $(objpfx)tst-printfsz-islongdouble.out \
  $(objpfx)tst-setvbuf1-cmp.out \
  $(objpfx)tst-unbputc.out \
  $(objpfx)tst-vfprintf-width-prec-mem.out \
  # tests-special

generated += \
  tst-printf-bz18872-mem.out \
  tst-printf-bz18872.c \
  tst-printf-bz18872.mtrace \
  tst-printf-bz25691-mem.out \
  tst-printf-bz25691.mtrace \
  tst-printf-fp-free-mem.out \
  tst-printf-fp-free.mtrace \
  tst-printf-fp-leak-mem.out \
  tst-printf-fp-leak.mtrace \
  tst-vfprintf-width-prec-mem.out \
  tst-vfprintf-width-prec.mtrace \
  # generated
endif # $(run-built-tests)

tests-special += $(objpfx)tst-errno-manual.out

include ../Rules

# The errlist.c is built in two phases because compiler might reorder the
# compat_symbol directive prior the object itself and on binutils older
# than 2.29 it might generate object sizes different than the expected ones.
$(objpfx)errlist-data-aux-shared.S: errlist-data-gen.c
	$(make-target-directory)
	$(compile-command.c) $(pic-cppflags) $(pic-ccflag) $(no-stack-protector) -S

$(objpfx)errlist-data-aux.S: errlist-data-gen.c
	$(make-target-directory)
	$(compile-command.c) $(pie-default) $(no-stack-protector) -S

$(objpfx)errlist-data.os: $(objpfx)errlist-data-aux-shared.S
$(objpfx)errlist-data.o: $(objpfx)errlist-data-aux.S

$(objpfx)siglist-aux-shared.S: siglist-gen.c
	$(make-target-directory)
	$(compile-command.c) $(pic-cppflags) $(pic-ccflag) $(no-stack-protector) -S

$(objpfx)siglist-aux.S: siglist-gen.c
	$(make-target-directory)
	$(compile-command.c) $(pie-default) $(no-stack-protector) -S

$(objpfx)siglist.os: $(objpfx)siglist-aux-shared.S
$(objpfx)siglist.o: $(objpfx)siglist-aux.S

ifeq ($(run-built-tests),yes)
LOCALES := \
  de_DE.ISO-8859-1 \
  de_DE.UTF-8 \
  en_US.ISO-8859-1 \
  hi_IN.UTF-8 \
  ja_JP.EUC-JP \
  ps_AF.UTF-8 \
  # LOCALES
include ../gen-locales.mk

$(objpfx)bug14.out: $(gen-locales)
$(objpfx)scanf13.out: $(gen-locales)
$(objpfx)test-vfprintf.out: $(gen-locales)
$(objpfx)tst-grouping.out: $(gen-locales)
$(objpfx)tst-sprintf.out: $(gen-locales)
$(objpfx)tst-sscanf.out: $(gen-locales)
$(objpfx)tst-swprintf.out: $(gen-locales)
$(objpfx)tst-vfprintf-mbs-prec.out: $(gen-locales)
$(objpfx)tst-vfprintf-width-i18n.out: $(gen-locales)
endif

tst-printf-bz18872-ENV = MALLOC_TRACE=$(objpfx)tst-printf-bz18872.mtrace \
			LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so
tst-vfprintf-width-prec-ENV = \
  MALLOC_TRACE=$(objpfx)tst-vfprintf-width-prec.mtrace \
  LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so
tst-printf-bz25691-ENV = \
  MALLOC_TRACE=$(objpfx)tst-printf-bz25691.mtrace \
  LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so
tst-printf-fp-free-ENV = \
  MALLOC_TRACE=$(objpfx)tst-printf-fp-free.mtrace \
  LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so
tst-printf-fp-leak-ENV = \
  MALLOC_TRACE=$(objpfx)tst-printf-fp-leak.mtrace \
  LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so

$(objpfx)tst-unbputc.out: tst-unbputc.sh $(objpfx)tst-unbputc
	$(SHELL) $< $(common-objpfx) '$(test-program-prefix)'; \
	$(evaluate-test)

$(objpfx)tst-printf.out: tst-printf.sh $(objpfx)tst-printf
	$(SHELL) $< $(common-objpfx) '$(test-program-prefix)'; \
	$(evaluate-test)

$(objpfx)tst-printfsz-islongdouble.out: \
  tst-printfsz-islongdouble.sh $(objpfx)tst-printfsz-islongdouble
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

# We generate this source because it requires a printf invocation with
# 10K arguments.
$(objpfx)tst-printf-bz18872.c: tst-printf-bz18872.sh
	rm -f $@ && $(BASH) $^ > $@.new && mv $@.new $@

$(objpfx)tst-%-mem.out: $(objpfx)tst-%.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-$*.mtrace > $@; \
	$(evaluate-test)

errlist-h = $(firstword $(wildcard $(addsuffix /errlist.h,$(sysdirs) .)))

$(objpfx)tst-errno-manual.out: tst-errno-manual.py \
			       $(errlist-h) \
			       $(..)manual/errno.texi
	$(PYTHON) tst-errno-manual.py -m $(..)manual/errno.texi \
				      -e $(errlist-h) > $@; \
	$(evaluate-test)

CFLAGS-vfprintf.c += -Wno-uninitialized
CFLAGS-vfwprintf.c += -Wno-uninitialized

CFLAGS-tmpfile.c += -fexceptions
CFLAGS-tmpfile64.c += -fexceptions
CFLAGS-tempname.c += -fexceptions
CFLAGS-psignal.c += -fexceptions
CFLAGS-vprintf.c += -fexceptions
CFLAGS-cuserid.c += -fexceptions

CFLAGS-vfprintf.c += -fexceptions
CFLAGS-fprintf.c += -fexceptions
CFLAGS-printf.c += -fexceptions
CFLAGS-vfwprintf.c += -fexceptions
CFLAGS-vfscanf.c += -fexceptions
CFLAGS-vfwscanf.c += -fexceptions
CFLAGS-fscanf.c += -fexceptions
CFLAGS-scanf.c += -fexceptions
CFLAGS-isoc99_vfscanf.c += -fexceptions
CFLAGS-isoc99_vscanf.c += -fexceptions
CFLAGS-isoc99_fscanf.c += -fexceptions
CFLAGS-isoc99_scanf.c += -fexceptions

# scanf14a.c and scanf16a.c test a deprecated extension which is no
# longer visible under most conformance levels; see the source files
# for more detail.
CFLAGS-scanf14a.c += -std=gnu89
CFLAGS-scanf16a.c += -std=gnu89

CFLAGS-bug3.c += -DOBJPFX=\"$(objpfx)\"
CFLAGS-bug4.c += -DOBJPFX=\"$(objpfx)\"
CFLAGS-bug5.c += -DOBJPFX=\"$(objpfx)\"
CFLAGS-test-fseek.c += -DOBJPFX=\"$(objpfx)\"
CFLAGS-test-popen.c += -DOBJPFX=\"$(objpfx)\"
CFLAGS-test_rdwr.c += -DOBJPFX=\"$(objpfx)\"

# tst-gets.c tests a deprecated function.
CFLAGS-tst-gets.c += -Wno-deprecated-declarations

# BZ #11319 was first fixed for regular vdprintf, then reopened because
# the fortified version had the same bug.
CFLAGS-tst-bz11319-fortify2.c += -D_FORTIFY_SOURCE=2

CFLAGS-tst-memstream-string.c += -fno-builtin-fprintf

CPPFLAGS += $(libio-mtsafe)

$(objpfx)tst-setvbuf1.out: /dev/null $(objpfx)tst-setvbuf1
	$(test-program-cmd) > $@ 2>&1; \
	$(evaluate-test)

$(objpfx)tst-setvbuf1-cmp.out: tst-setvbuf1.expect $(objpfx)tst-setvbuf1.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)tst-printf-round: $(libm)
$(objpfx)tst-scanf-round: $(libm)
