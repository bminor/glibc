ifeq ($(subdir),math)
# branred depends on precise IEEE double rounding
CFLAGS-branred.c += $(config-cflags-nofma)
CFLAGS-e_sqrt.c += $(config-cflags-nofma)

# The symbols alias to lround
CFLAGS-s_llround.c += -fno-builtin-lround
endif

ifeq ($(subdir),stdio-common)
fmt-xscanf-real-convs += double
endif
