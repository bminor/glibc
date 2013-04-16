ifeq ($(subdir),math)
sysdep_routines += s_isnan-power7 s_isnan-power6 s_isnan-power5 \
		   s_isnan-ppc32 \
		   s_isnanf-power7 s_isnanf-power6 s_isnanf-power5 \
		   s_isnanf-ppc32 \
		   s_copysign-power6 s_copysign-ppc32 \
		   s_finite-power7 s_finite-c \
		   s_finitef-c \
		   s_isinf-power7 s_isinf-c \
		   s_isinff-c

libm-sysdep_routines += s_llrintf-power6 s_llrintf-power4 s_llrintf-c \
			s_llrint-power6 s_llrint-power4 s_llrint-c \
			s_llround-power6 s_llround-power5+ s_llround-power4 \
			s_llround-c \
			w_sqrt-power5 w_sqrt-power4 w_sqrt-c \
			w_sqrtf-power5 w_sqrtf-power4 w_sqrtf-c \
			s_isnan-power7 s_isnan-power6 s_isnan-power5 \
			s_isnan-ppc32 \
			s_isnanf-power7 s_isnanf-power6 s_isnanf-power5 \
			s_isnanf-ppc32 \
			s_copysign-power6 s_copysign-ppc32 \
			s_ceil-power5+ s_ceil-ppc32 \
			s_ceilf-power5+ s_ceilf-ppc32 \
			s_floor-power5+ s_floor-ppc32 \
			s_floorf-power5+ s_floorf-ppc32 \
			s_round-power5+ s_round-ppc32 \
			s_roundf-power5+ s_roundf-ppc32 \
			s_trunc-power5+ s_trunc-ppc32 \
			s_truncf-power5+ s_truncf-ppc32 \
			s_lround-power6x s_lround-power5+ s_lround-ppc32 \
			s_lrint-power6x s_lrint-ppc32 \
			s_finite-power7 s_finite-c \
			s_finitef-c \
			s_isinf-power7 s_isinf-c \
			s_isinff-c \
			s_logbl-power7 s_logbl-c \
			s_logb-power7 s_logb-c \
			s_logbf-power7 s_logbf-c

CFLAGS-s_llround.c = -fno-builtin-llroundf
CFLAGS-s_lround.c = -fno-builtin-lroundf
endif
