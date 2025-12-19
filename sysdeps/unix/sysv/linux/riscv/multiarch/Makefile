ifeq ($(subdir),string)
sysdep_routines += \
  memcpy \
  memcpy-generic \
  memcpy_noalignment \
  memset \
  memset-generic \
  memset-vector \
  # sysdep_routines

CFLAGS-memcpy_noalignment.c += -mno-strict-align
endif
