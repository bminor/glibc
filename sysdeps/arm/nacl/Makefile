# XXX temporary until the compiler build can be fixed
sysdep-CPPFLAGS += -D__ARM_NEON__=1

ifeq ($(subdir),csu)
sysdep_routines += aeabi_read_tp
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += aeabi_read_tp
endif
