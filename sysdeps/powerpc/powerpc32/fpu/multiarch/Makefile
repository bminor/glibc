ifeq ($(subdir),math)
sysdep_routines += s_isnan-power7 s_isnan-power6 s_isnan-power5 \
		   s_isnan-ppc32 \
		   s_isnanf-power7 s_isnanf-power6 s_isnanf-power5 \
		   s_isnanf-ppc32 \
		   s_copysign-power6 s_copysign-ppc32

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
			s_copysign-power6 s_copysign-ppc32

CFLAGS-s_llround.c = -fno-builtin-llroundf
endif
