# We don't really support TLSDESC, even though the compiler thinks it does.
have-arm-tls-desc = no

ifeq ($(subdir),csu)
sysdep_routines += aeabi_read_tp
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += aeabi_read_tp
endif
