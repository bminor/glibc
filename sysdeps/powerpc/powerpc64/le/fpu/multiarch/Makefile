ifeq ($(subdir),math)
libm-sysdep_routines += s_fmaf128-ppc64 s_fmaf128-power9 \
			w_sqrtf128-power9 w_sqrtf128-ppc64le

CFLAGS-s_fmaf128-ppc64.c += $(type-float128-CFLAGS) $(no-gnu-attribute-CFLAGS)
CFLAGS-s_fmaf128-power9.c += $(type-float128-CFLAGS) -mcpu=power9 $(no-gnu-attribute-CFLAGS)

CFLAGS-w_sqrtf128-ppc64le.c += $(type-float128-CFLAGS) $(no-gnu-attribute-CFLAGS)
CFLAGS-w_sqrtf128-power9.c += $(type-float128-CFLAGS) -mcpu=power9 $(no-gnu-attribute-CFLAGS)
endif
