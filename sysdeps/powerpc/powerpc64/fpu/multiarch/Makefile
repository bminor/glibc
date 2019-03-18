ifeq ($(subdir),math)
sysdep_routines += $(sysdep_calls)
libm-sysdep_routines += s_logb-power7 s_logbf-power7 \
			s_logbl-power7 s_logb-ppc64 s_logbf-ppc64 \
			s_logbl-ppc64 \
			$(sysdep_calls:s_%=m_%)

CFLAGS-s_logbf-power7.c = -mcpu=power7
CFLAGS-s_logbl-power7.c = -mcpu=power7
CFLAGS-s_logb-power7.c = -mcpu=power7
endif
