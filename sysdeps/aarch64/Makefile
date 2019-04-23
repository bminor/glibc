long-double-fcts = yes

ifeq ($(subdir),elf)
sysdep-dl-routines += tlsdesc dl-tlsdesc
gen-as-const-headers += dl-link.sym

tests-internal += tst-ifunc-arg-1 tst-ifunc-arg-2
endif

ifeq ($(subdir),csu)
gen-as-const-headers += tlsdesc.sym
endif

ifeq ($(subdir),gmon)
CFLAGS-mcount.c += -mgeneral-regs-only
endif

ifeq ($(subdir),math)
CPPFLAGS += -I../soft-fp
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/ifunc.h
endif
