ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

abi-variants := lp64
abi-variants += lp64_be
abi-variants += purecap
abi-variants += purecap_be

ifeq (,$(filter $(default-abi),$(abi-variants)))
Unknown ABI, must be one of $(abi-variants)
endif

abi-lp64-condition := !defined __AARCH64EB__ && !defined __CHERI_PURE_CAPABILITY__
abi-lp64_be-condition := defined __AARCH64EB__ && !defined __CHERI_PURE_CAPABILITY__
abi-purecap-condition := !defined __AARCH64EB__ && defined __CHERI_PURE_CAPABILITY__
abi-purecap_be-condition := defined __AARCH64EB__ && defined __CHERI_PURE_CAPABILITY__
