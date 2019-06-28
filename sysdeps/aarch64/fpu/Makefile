ifeq ($(subdir),math)
CFLAGS-e_sqrtf.c += -fno-math-errno
CFLAGS-e_sqrt.c += -fno-math-errno
CFLAGS-s_lroundf.c += -fno-math-errno
CFLAGS-s_lround.c += -fno-math-errno
CFLAGS-s_llroundf.c += -fno-math-errno
CFLAGS-s_llround.c += -fno-math-errno
# GCC 4.9 and 5 requires the flag to correct emits a f{max,min}nm
# for a __builtin_{fmax,fmin}{f}.
CFLAGS-s_fmax.c += -ffinite-math-only
CFLAGS-s_fmaxf.c += -ffinite-math-only
CFLAGS-s_fmin.c += -ffinite-math-only
CFLAGS-s_fminf.c += -ffinite-math-only
endif

ifeq ($(subdir),mathvec)
libmvec-support += \
  libmvec_double_vlen2_exp \
  libmvec_float_vlen4_expf \

install-lib += libmvec_nonshared.a
$(objpfx)libmvec_nonshared.a: $(dep-dummy-lib); $(make-dummy-lib)
endif

ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
double-vlen2-funcs = exp
float-vlen4-funcs = exp
ifeq ($(test-mathvec),yes)
libmvec-tests += double-vlen2 float-vlen4
endif
endif
endif
