# When building float128 we need to ensure -mfloat128 is
# passed to all such object files.

# Bootstrapping code for enabling IEEE 128.  This can be removed and
# any indirections simplified once IEEE 128 long double is enabled.
type-ldouble-CFLAGS =
no-gnu-attribute-CFLAGS =
ifeq ($(ibm128-fcts),yes)
type-ldouble-CFLAGS += -mabi=ibmlongdouble
no-gnu-attribute-CFLAGS = -mno-gnu-attribute
endif

ifeq ($(subdir),math)
# sqrtf128 requires emulation before POWER9.
CPPFLAGS += -I../soft-fp

# float128 requires adding a handful of extra flags.
# Similarly, disable
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),%f128_r$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float128%$(suf)): CFLAGS += -mfloat128
$(foreach suf,$(all-object-suffixes),$(objpfx)test-float64x%$(suf)): CFLAGS += -mfloat128
# Pairs of types with _Float128 / _Float64x as the wider type but not
# the narrower one.
f128-pairs = float32-float64x float32-float128 float64-float64x \
	     float64-float128 float32x-float64x float32x-float128
$(foreach suf,$(all-object-suffixes),$(foreach pair,$(f128-pairs),$(objpfx)test-$(pair)%$(suf))): CFLAGS += -mfloat128

CFLAGS-libm-test-support-float128.c += -mfloat128
CFLAGS-libm-test-support-float64x.c += -mfloat128
CFLAGS-test-math-iscanonical.cc += -mfloat128
CFLAGS-test-math-iseqsig.cc += -mfloat128
CFLAGS-test-math-issignaling.cc += -mfloat128
CFLAGS-test-math-iszero.cc += -mfloat128

CFLAGS-s_logbl-power7.c += $(type-ldouble-CFLAGS)
CFLAGS-s_logbl-ppc64.c += $(type-ldouble-CFLAGS)

$(foreach suf,$(all-object-suffixes),\
          $(objpfx)s_copysignl$(suf) \
          $(objpfx)s_fabsl$(suf)): \
  ASFLAGS += $(type-ldouble-CFLAGS)

$(foreach suf,$(all-object-suffixes),\
         $(objpfx)libm-test-%ibm128$(suf) \
         $(objpfx)test-iibm128%$(suf) $(objpfx)test-ibm128%$(suf)): \
  CFLAGS += $(type-ldouble-CFLAGS)

# Newer GCC (>7) doesn't like -mabi=* and -mlong-double-64
$(foreach suf,$(all-object-suffixes),\
         $(objpfx)test-narrow-macros-ldbl-64$(suf) \
         $(objpfx)test-nldbl-redirect$(suf) \
         $(objpfx)test-redirection-ldbl-64$(suf) \
         ): sysdep-CFLAGS := $(filter-out -mabi=ieeelongdouble,$(sysdep-CFLAGS))

endif

# Newer GCC (>7) doesn't like -mabi=* and -mlong-double-64
ifeq ($(subdir),misc)
$(foreach suf,$(all-object-suffixes),\
         $(objpfx)tst-nldbl-warn$(suf) \
         $(objpfx)tst-nldbl-error$(suf) \
         ): sysdep-CFLAGS := $(filter-out -mabi=ieeelongdouble,$(sysdep-CFLAGS))
endif

# Newer GCC (>7) doesn't like -mabi=* and -mlong-double-64
ifeq ($(subdir),argp)
$(foreach suf,$(all-object-suffixes),\
         $(objpfx)tst-nldbl-argp$(suf) \
         ): sysdep-CFLAGS := $(filter-out -mabi=ieeelongdouble,$(sysdep-CFLAGS))
endif

# Append flags to string <-> _Float128 routines.
ifneq ($(filter $(subdir),wcsmbs stdlib),)
$(foreach suf,$(all-object-suffixes),%f128$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),%f128_l$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),%f128_nan$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),%float1282mpn$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
$(foreach suf,$(all-object-suffixes),%mpn2float128$(suf)): CFLAGS += -mfloat128 $(no-gnu-attribute-CFLAGS)
CFLAGS-bug-strtod.c += -mfloat128
CFLAGS-bug-strtod2.c += -mfloat128
CFLAGS-tst-strtod-round.c += -mfloat128
CFLAGS-tst-wcstod-round.c += -mfloat128
CFLAGS-tst-strtod-nan-locale.c += -mfloat128
CFLAGS-tst-wcstod-nan-locale.c += -mfloat128
CFLAGS-tst-strtod6.c += -mfloat128
CFLAGS-tst-strfrom.c += -mfloat128
CFLAGS-tst-strfrom-locale.c += -mfloat128
CFLAGS-strfrom-skeleton.c += -mfloat128
CFLAGS-tst-strtod-nan-sign.c += -mfloat128
CFLAGS-tst-wcstod-nan-sign.c += -mfloat128

# When building glibc with support for _Float128, the powers of ten tables in
# fpioconst.c and in the string conversion functions must be extended.  Some
# Makefiles (e.g.: wcsmbs/Makefile) override CFLAGS defined by the Makefiles in
# sysdeps.  This is avoided with the use sysdep-CFLAGS instead of CFLAGS.
sysdep-CFLAGS += $(sysdep-CFLAGS-$(<F))
sysdep-CFLAGS-fpioconst.c += -mfloat128
sysdep-CFLAGS-strtod_l.c += -mfloat128
sysdep-CFLAGS-strtof_l.c += -mfloat128
sysdep-CFLAGS-strtold_l.c += -mfloat128
sysdep-CFLAGS-wcstod_l.c += -mfloat128
sysdep-CFLAGS-wcstof_l.c += -mfloat128
sysdep-CFLAGS-wcstold_l.c += -mfloat128
endif

# Append flags to printf routines.
ifeq ($(subdir),stdio-common)
CFLAGS-printf_fp.c = -mfloat128
CFLAGS-printf_fphex.c = -mfloat128
CFLAGS-printf_size.c = -mfloat128
endif

# Disable linker noise on files using ieee128 long double internally
ifeq ($(subdir),time)
CFLAGS-difftime.c += $(no-gnu-attribute-CFLAGS)
endif

$(foreach suf,$(all-object-suffixes),$(objpfx)nldbl-%$(suf)): \
  CFLAGS += $(type-ldouble-CFLAGS)

ldbl-ibm128-files = $(objpfx)nldbl-%$(suf) \
		    $(objpfx)libm-test-%ibm128$(suf) \
		    $(objpfx)test-iibm128%$(suf) $(objpfx)test-ibm128%$(suf) \
		    $(objpfx)test-tgmath3-%$(suf)

# Remove -mabi=ieeelongdouble from ldbl-ibm128 files.
$(foreach suf,$(all-object-suffixes), $(ldbl-ibm128-files)) \
	  $(foreach r,$(ldbl-128ibm-routines) $(ldbl-tests), \
		    $(objpfx)$(r)$(suf)): \
  sysdep-CFLAGS := $(filter-out -mabi=ieeelongdouble,$(sysdep-CFLAGS))
