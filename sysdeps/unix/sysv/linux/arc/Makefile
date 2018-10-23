ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),signal)
sysdep_routines += sigrestorer
endif

ifeq ($(subdir),misc)
# MIPS/Tile-style cacheflush routine
sysdep_headers += sys/cachectl.h
sysdep_routines += cacheflush
endif

ifeq ($(subdir),elf)
ifeq ($(build-shared),yes)
# This is needed for DSO loading from static binaries.
sysdep-dl-routines += dl-static
endif
endif

abi-variants := arcle arcbe

ifeq (,$(filter $(default-abi),$(abi-variants)))
$(error Unknown ABI $(default-abi), must be one of $(abi-variants))
endif

abi-arcle-condition := !defined __BIG_ENDIAN__
abi-arcbe-condition := defined __BIG_ENDIAN__
