# When building float128 we need to ensure -mfloat128 is
# passed to all such object files.

float128-flags = -mfloat128
ifeq ($(with-clang),yes)
float128-flags =
#Enable this when we know we are targeting power9
#float128-flags += -mcpu=pwr9
endif

# libgcc requires __tcb_parse_hwcap_and_convert_at_platform when built with
# a binary128 type.  That symbol is provided by the loader on dynamically
# linked executables, forcing to link the loader after libgcc link.
f128-loader-link = $(as-needed) $(elf-objpfx)ld.so $(no-as-needed)

ifeq ($(subdir),math)
# sqrtf128 requires emulation before POWER9.
CPPFLAGS += -I../soft-fp

# float128 requires adding a handful of extra flags.
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),%f128_r$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float128%$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float64x%$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),$(objpfx)test-ifloat128%$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),$(objpfx)test-ifloat64x%$(suf)): CFLAGS += $(float128-flags)
CFLAGS-libm-test-support-float128.c += $(float128-flags)
CFLAGS-libm-test-support-float64x.c += $(float128-flags)
CFLAGS-test-math-iscanonical.cc += $(float128-flags)
CFLAGS-test-math-iseqsig.cc += $(float128-flags)
CFLAGS-test-math-issignaling.cc += $(float128-flags)
CFLAGS-test-math-iszero.cc += $(float128-flags)
$(foreach test, \
	  test-float128% test-ifloat128% test-float64x% test-ifloat64x% \
	  test-math-iscanonical test-math-iseqsig test-math-issignaling \
	  test-math-iszero, \
	  $(objpfx)$(test)): \
  gnulib-tests += $(f128-loader-link)
endif

# Append flags to string <-> _Float128 routines.
ifneq ($(filter $(subdir),wcsmbs stdlib),)
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),%f128_l$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),%f128_nan$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),%float1282mpn$(suf)): CFLAGS += $(float128-flags)
$(foreach suf,$(all-object-suffixes),%mpn2float128$(suf)): CFLAGS += $(float128-flags)
CFLAGS-bug-strtod.c += $(float128-flags)
CFLAGS-bug-strtod2.c += $(float128-flags)
CFLAGS-tst-strtod-round.c += $(float128-flags)
CFLAGS-tst-wcstod-round.c += $(float128-flags)
CFLAGS-tst-strtod-nan-locale.c += $(float128-flags)
CFLAGS-tst-wcstod-nan-locale.c += $(float128-flags)
CFLAGS-tst-strtod6.c += $(float128-flags)
CFLAGS-tst-strfrom.c += $(float128-flags)
CFLAGS-tst-strfrom-locale.c += $(float128-flags)
CFLAGS-strfrom-skeleton.c += $(float128-flags)
$(foreach test,bug-strtod bug-strtod2 bug-strtod2 tst-strtod-round \
tst-wcstod-round tst-strtod6 tst-strrom tst-strfrom-locale \
tst-strtod-nan-locale tst-wcstod-nan-locale \
strfrom-skeleton,$(objpfx)$(test)): gnulib-tests += $(f128-loader-link)

# When building glibc with support for _Float128, the powers of ten tables in
# fpioconst.c and in the string conversion functions must be extended.  Some
# Makefiles (e.g.: wcsmbs/Makefile) override CFLAGS defined by the Makefiles in
# sysdeps.  This is avoided with the use sysdep-CFLAGS instead of CFLAGS.
sysdep-CFLAGS += $(sysdep-CFLAGS-$(<F))
#  -mllvm -enable-ppc-quad-precision
sysdep-CFLAGS-fpioconst.c += $(float128-flags)
sysdep-CFLAGS-strtod_l.c += $(float128-flags)
sysdep-CFLAGS-strtof_l.c += $(float128-flags)
sysdep-CFLAGS-strtold_l.c += $(float128-flags)
sysdep-CFLAGS-wcstod_l.c += $(float128-flags)
sysdep-CFLAGS-wcstof_l.c += $(float128-flags)
sysdep-CFLAGS-wcstold_l.c += $(float128-flags)
endif

# Append flags to printf routines.
ifeq ($(subdir),stdio-common)
CFLAGS-printf_fp.c = $(float128-flags)
CFLAGS-printf_fphex.c = $(float128-flags)
CFLAGS-printf_size.c = $(float128-flags)
endif
