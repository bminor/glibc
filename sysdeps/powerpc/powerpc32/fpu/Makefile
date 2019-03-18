ifeq ($(subdir),math)
# lrint is aliased to lrintf, so suppress compiler builtins to
# avoid mismatched signatures.
CFLAGS-s_lrint.c += -fno-builtin-lrintf
endif

ifeq ($(subdir),misc)
sysdep_routines += fprsave fprrest
endif
