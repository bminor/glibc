gnulib-arch = $(elf-objpfx)libgcc-stubs.a
static-gnulib-arch = $(elf-objpfx)libgcc-stubs.a

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
sysdep_routines += aeabi_unwind_cpp_pr1 find_exidx
sysdep-rtld-routines += aeabi_unwind_cpp_pr1
shared-only-routines += aeabi_unwind_cpp_pr1

$(objpfx)libgcc-stubs.a: $(objpfx)aeabi_unwind_cpp_pr1.os
	$(build-extra-lib)

lib-noranlib: $(objpfx)libgcc-stubs.a

ifeq ($(build-shared),yes)
ifneq (no,$(have-test-mtls-descriptor))
tests += tst-armtlsdescloc tst-armtlsdescextnow tst-armtlsdescextlazy
modules-names += tst-armtlsdesclocmod
modules-names += tst-armtlsdescextlazymod tst-armtlsdescextnowmod
CPPFLAGS-tst-armtlsdescextnowmod.c += -Dstatic=
CPPFLAGS-tst-armtlsdescextlazymod.c += -Dstatic=
CFLAGS-tst-armtlsdesclocmod.c += -mtls-dialect=$(have-test-mtls-descriptor)
CFLAGS-tst-armtlsdescextnowmod.c += -mtls-dialect=$(have-test-mtls-descriptor)
CFLAGS-tst-armtlsdescextlazymod.c += -mtls-dialect=$(have-test-mtls-descriptor)
LDFLAGS-tst-armtlsdescextnowmod.so += -Wl,-z,now
tst-armtlsdescloc-ENV = LD_BIND_NOW=1
tst-armtlsdescextnow-ENV = LD_BIND_NOW=1
tst-armtlsdescextlazy-ENV = LD_BIND_NOW=1
$(objpfx)tst-armtlsdescloc: $(objpfx)tst-armtlsdesclocmod.so
$(objpfx)tst-armtlsdescextnow: $(objpfx)tst-armtlsdescextnowmod.so
$(objpfx)tst-armtlsdescextlazy: $(objpfx)tst-armtlsdescextlazymod.so
endif
endif
endif

ifeq ($(subdir),csu)
# get offset to rtld_global._dl_hwcap
gen-as-const-headers += rtld-global-offsets.sym tlsdesc.sym
aeabi_constants = aeabi_lcsts aeabi_sighandlers aeabi_math
aeabi_routines = aeabi_assert aeabi_localeconv aeabi_errno_addr \
		 aeabi_mb_cur_max aeabi_atexit aeabi_memclr aeabi_memcpy \
		 aeabi_memmove aeabi_memset \
		 aeabi_read_tp libc-aeabi_read_tp

sysdep_routines += $(aeabi_constants) $(aeabi_routines)
static-only-routines += $(aeabi_constants) aeabi_read_tp
shared-only-routines += libc-aeabi_read_tp

sysdep_routines += arm-unwind-resume
shared-only-routines += arm-unwind-resume
endif

ifeq ($(subdir),gmon)
sysdep_routines += arm-mcount
endif

ifeq ($(subdir),math)
CFLAGS-feupdateenv.c += $(config-cflags-wno-ignored-attributes)
endif

ifeq ($(subdir),rt)
librt-sysdep_routines += rt-aeabi_unwind_cpp_pr1 rt-arm-unwind-resume
librt-shared-only-routines += rt-aeabi_unwind_cpp_pr1 rt-arm-unwind-resume
endif
