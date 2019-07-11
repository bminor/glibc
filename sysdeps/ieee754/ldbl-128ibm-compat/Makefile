ifeq ($(subdir),libio)
ldbl-extra-routines += fwscanf \
		       swscanf \
		       wscanf \
		       vswscanf \
		       vwscanf
endif

ifeq ($(subdir),stdio-common)
ldbl-extra-routines += printf_size \
		       asprintf \
		       dprintf \
		       fprintf \
		       printf \
		       snprintf \
		       sprintf \
		       vasprintf \
		       vdprintf \
		       vfprintf \
		       vprintf \
		       vsnprintf \
		       vsprintf \
		       fwprintf \
		       swprintf \
		       wprintf \
		       vfwprintf \
		       vswprintf \
		       vwprintf \
		       fscanf \
		       scanf \
		       sscanf \
		       vfscanf \
		       vscanf \
		       vsscanf \
		       vfwscanf

# Printing long double values with IEEE binary128 format reuses part
# of the internal float128 implementation (__printf_fp, __printf_fphex,
# and __float128 variables and union members).  Likewise, reading these
# values reuses __strtof128_internal.  Thus, the compilation of the
# following functions, must have -mfloat128 and -mabi=ibmlongdouble
# passed to the compiler.
CFLAGS-vfprintf-internal.c += -mfloat128 -mabi=ibmlongdouble
CFLAGS-vfwprintf-internal.c += -mfloat128 -mabi=ibmlongdouble
CFLAGS-vfscanf-internal.c += -mfloat128 -mabi=ibmlongdouble
CFLAGS-vfwscanf-internal.c += -mfloat128 -mabi=ibmlongdouble

# Basic tests for the implementation of long double with IEEE binary128
# format and for the related redirections in installed headers.
tests-internal += test-printf-ieee128 test-printf-ibm128
CFLAGS-test-printf-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-printf-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

tests-internal += test-wprintf-ieee128 test-wprintf-ibm128
CFLAGS-test-wprintf-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-wprintf-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

tests-internal += test-scanf-ieee128 test-scanf-ibm128
CFLAGS-test-scanf-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-scanf-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

$(objpfx)test-scanf-ieee128: gnulib-tests += $(f128-loader-link)

tests-internal += test-wscanf-ieee128 test-wscanf-ibm128
CFLAGS-test-wscanf-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-wscanf-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

$(objpfx)test-wscanf-ieee128: gnulib-tests += $(f128-loader-link)

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)test-scanf-ieee128.out
tests-special += $(objpfx)test-scanf-ibm128.out

tests-special += $(objpfx)test-wscanf-ieee128.out
tests-special += $(objpfx)test-wscanf-ibm128.out
endif

$(objpfx)test-scanf-ieee128.out: \
  ../sysdeps/ieee754/ldbl-128ibm-compat/test-scanf-ldbl-compat.sh \
  $(objpfx)test-scanf-ieee128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

$(objpfx)test-scanf-ibm128.out: \
  ../sysdeps/ieee754/ldbl-128ibm-compat/test-scanf-ldbl-compat.sh \
  $(objpfx)test-scanf-ibm128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

$(objpfx)test-wscanf-ieee128.out: \
  ../sysdeps/ieee754/ldbl-128ibm-compat/test-scanf-ldbl-compat.sh \
  $(objpfx)test-wscanf-ieee128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

$(objpfx)test-wscanf-ibm128.out: \
  ../sysdeps/ieee754/ldbl-128ibm-compat/test-scanf-ldbl-compat.sh \
  $(objpfx)test-wscanf-ibm128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

tests-internal += test-printf-size-ieee128 test-printf-size-ibm128
CFLAGS-test-printf-size-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-printf-size-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)test-printf-size-ieee128.out
tests-special += $(objpfx)test-printf-size-ibm128.out
endif

$(objpfx)test-printf-size-ieee128.out: \
  tst-printfsz-islongdouble.sh $(objpfx)test-printf-size-ieee128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)

$(objpfx)test-printf-size-ibm128.out: \
  tst-printfsz-islongdouble.sh $(objpfx)test-printf-size-ibm128
	$(SHELL) $^ '$(test-program-prefix)' $@; \
	$(evaluate-test)
endif

ifeq ($(subdir),debug)
ldbl-extra-routines += asprintf_chk \
		       dprintf_chk \
		       fprintf_chk \
		       printf_chk \
		       snprintf_chk \
		       sprintf_chk \
		       vasprintf_chk \
		       vdprintf_chk \
		       vfprintf_chk \
		       vprintf_chk \
		       vsnprintf_chk \
		       vsprintf_chk \
		       fwprintf_chk \
		       swprintf_chk \
		       wprintf_chk \
		       vfwprintf_chk \
		       vswprintf_chk \
		       vwprintf_chk

tests-internal += test-printf-chk-ieee128 test-printf-chk-ibm128
CFLAGS-test-printf-chk-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-printf-chk-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

tests-internal += test-printf-chk-redir-ieee128 test-printf-chk-redir-ibm128
CFLAGS-test-printf-chk-redir-ieee128.c += -mfloat128 -mabi=ieeelongdouble \
					  -Wno-psabi -Wno-unused-result
CFLAGS-test-printf-chk-redir-ibm128.c += -mabi=ibmlongdouble -Wno-psabi \
					 -Wno-unused-result

tests-internal += test-wprintf-chk-ieee128 test-wprintf-chk-ibm128
CFLAGS-test-wprintf-chk-ieee128.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-test-wprintf-chk-ibm128.c += -mabi=ibmlongdouble -Wno-psabi

tests-internal += test-wprintf-chk-redir-ieee128 test-wprintf-chk-redir-ibm128
CFLAGS-test-wprintf-chk-redir-ieee128.c += -mfloat128 -mabi=ieeelongdouble \
					   -Wno-psabi -Wno-unused-result
CFLAGS-test-wprintf-chk-redir-ibm128.c += -mabi=ibmlongdouble -Wno-psabi \
					  -Wno-unused-result
endif

ifeq ($(subdir),argp)
ldbl-extra-routines += argp-help

$(objpfx)tst-ieee128-argp.c: tst-ldbl-argp.c
	cp $< $@

$(objpfx)tst-ibm128-argp.c: tst-ldbl-argp.c
	cp $< $@

tests-internal += tst-ieee128-argp tst-ibm128-argp
CFLAGS-tst-ieee128-argp.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-tst-ibm128-argp.c += -mabi=ibmlongdouble -Wno-psabi
endif

ifeq ($(subdir),misc)
ldbl-extra-routines += err \
		       error

tests-internal += tst-ibm128-warn tst-ieee128-warn
tests-internal += tst-ibm128-error tst-ieee128-error

$(objpfx)tst-ibm128-%.c: tst-ldbl-%.c
	cp $< $@

$(objpfx)tst-ieee128-%.c: tst-ldbl-%.c
	cp $< $@

CFLAGS-tst-ibm128-warn.c += -mabi=ibmlongdouble -Wno-psabi
CFLAGS-tst-ibm128-error.c += -mabi=ibmlongdouble -Wno-psabi

CFLAGS-tst-ieee128-warn.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
CFLAGS-tst-ieee128-error.c += -mfloat128 -mabi=ieeelongdouble -Wno-psabi
endif

# Add IEEE binary128 files as make targets.
routines += $(foreach r,$(ldbl-extra-routines),ieee128-$(r))

# On powerpc64le, the routines responsible for converting between long
# double and string (e.g.: printf, scanf, strtold, strfroml) default to
# IBM long double mode.  When support for TS 18661-3 was added, several
# internal functions were modified so that they could also handle
# floating-point variables with IEEE binary128 format, but as an
# additional type, _Float128.
#
# The following rule ensures that the string conversion routines will
# always be built in IBM long double mode, with additional support for
# IEEE binary128, through the use of -mabi=ibmlongdouble and -mfloat128.
ldbl-ibm128-files := $(objpfx)test-%-ibm128^ \
		     $(foreach r,$(ldbl-extra-routines),$(objpfx)$(r)^) \
		     $(foreach r,$(ldbl-extra-routines),$(objpfx)$(r)-internal^)
obj-suf-foreach = $(foreach suf,$(all-object-suffixes),$(subst ^,$(suf),$(1)))
$(call obj-suf-foreach,$(ldbl-ibm128-files)): \
  sysdep-CFLAGS += -mabi=ibmlongdouble
