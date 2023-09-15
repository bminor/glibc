ifeq ($(subdir),misc)
sysdep_headers += sys/asm.h

tests += \
  tst-hwcap-tunables \
  # tests

tst-hwcap-tunables-ARGS = -- $(host-test-program-cmd)
endif

ifeq ($(subdir),elf)
gen-as-const-headers += dl-link.sym
endif

ifeq ($(subdir),elf)
sysdep-dl-routines += \
  dl-get-cpu-features \
  # sysdep-dl-routines
endif

# LoongArch's assembler also needs to know about PIC as it changes the
# definition of some assembler macros.
ASFLAGS-.os += $(pic-ccflag)

# All the objects in lib*_nonshared.a need to be compiled with medium code
# model or large applications may fail to link.
ifeq (yes,$(have-cmodel-medium))
CFLAGS-.oS += -mcmodel=medium
endif
