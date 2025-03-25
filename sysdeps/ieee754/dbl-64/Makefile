ifeq ($(subdir),math)
# branred depends on precise IEEE double rounding
CFLAGS-branred.c += $(config-cflags-nofma)
CFLAGS-e_sqrt.c += $(config-cflags-nofma)
endif

ifeq ($(subdir),stdio-common)
fmt-xscanf-real-convs += double
endif
