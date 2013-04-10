ifeq ($(subdir),math)
libm-sysdep_routines += s_llrintf-power6 s_llrintf-power4 s_llrintf-c \
			s_llrint-power6 s_llrint-power4 s_llrint-c \
			s_llround-power6 s_llround-power5+ s_llround-power4 \
			s_llround-c \
			w_sqrt-power5 w_sqrt-power4 w_sqrt-c

CFLAGS-s_llround.c = -fno-builtin-llroundf
endif
