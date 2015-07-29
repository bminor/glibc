ifeq ($(subdir),csu)
gen-as-const-headers += cpu-features-offsets.sym rtld-global-offsets.sym
endif

ifeq ($(subdir),elf)
sysdep-ld-routines += dl-get-cpu-features
sysdep-rtld-routines += dl-get-cpu-features
sysdep_routines += dl-get-cpu-features
elide-routines.os += dl-get-cpu-features

tests += tst-get-cpu-features
tests-static += tst-get-cpu-features-static
endif
