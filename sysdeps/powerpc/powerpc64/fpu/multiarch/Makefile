ifeq ($(subdir),math)
# These functions are built both for libc and libm because they're required
# by printf.  While the libc objects have the prefix s_, the libm ones are
# prefixed with  m_.
sysdep_calls := s_finite-power8 s_finite-power7 s_finite-ppc64 \
		s_finitef-ppc64 \
		s_modf-power5+ s_modf-ppc64 \
		s_modff-power5+ s_modff-ppc64

sysdep_routines += $(sysdep_calls)
libm-sysdep_routines += s_llround-power6x \
			s_llround-power5+ s_llround-ppc64 \
			s_llrint-power6x s_llrint-ppc64 \
			s_logb-power7 s_logbf-power7 \
			s_logbl-power7 s_logb-ppc64 s_logbf-ppc64 \
			s_logbl-ppc64 e_hypot-ppc64 \
			e_hypot-power7 e_hypotf-ppc64 e_hypotf-power7 \
			s_llrint-power8 s_llround-power8 s_llroundf-ppc64 \
			e_expf-power8 e_expf-ppc64 \
			$(sysdep_calls:s_%=m_%)

CFLAGS-s_logbf-power7.c = -mcpu=power7
CFLAGS-s_logbl-power7.c = -mcpu=power7
CFLAGS-s_logb-power7.c = -mcpu=power7
CFLAGS-s_modf-power5+.c = -mcpu=power5+
CFLAGS-s_modff-power5+.c = -mcpu=power5+
CFLAGS-e_hypot-power7.c = -mcpu=power7
CFLAGS-e_hypotf-power7.c = -mcpu=power7

# These files quiet sNaNs in a way that is optimized away without
# -fsignaling-nans.
CFLAGS-s_modf-ppc64.c += -fsignaling-nans
CFLAGS-s_modff-ppc64.c += -fsignaling-nans
endif
