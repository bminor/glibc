ifeq ($(subdir),mathvec)
libmvec-sysdep_routines += vec_d_cos2_vsx
CFLAGS-vec_d_cos2_vsx.c += -mabi=altivec -maltivec -mvsx
endif

# Variables for libmvec tests.
ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
libmvec-tests += double-vlen2

double-vlen2-funcs = cos

double-vlen2-arch-ext-cflags = -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX
CFLAGS-test-double-vlen2-wrappers.c += -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX
endif
endif
