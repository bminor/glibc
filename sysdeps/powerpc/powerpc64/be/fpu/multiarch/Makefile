ifeq ($(subdir),math)
libm-sysdep_routines += s_ceil-power5+ \
			s_ceil-ppc64 \
			s_ceilf-power5+ \
			s_ceilf-ppc64 \
			s_floor-power5+ \
			s_floor-ppc64 \
			s_floorf-power5+ \
			s_floorf-ppc64 \
			s_round-power5+ \
			s_round-ppc64 \
			s_roundf-power5+ \
			s_roundf-ppc64

CFLAGS-s_ceil-power5+.c = -mcpu=power5+
CFLAGS-s_ceilf-power5+.c = -mcpu=power5+
CFLAGS-s_floor-power5+.c = -mcpu=power5+
CFLAGS-s_floorf-power5+.c = -mcpu=power5+
CFLAGS-s_round-power5+.c = -mcpu=power5+
CFLAGS-s_roundf-power5+.c = -mcpu=power5+
endif
