ifeq ($(subdir),string)
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strspn-c
CFLAGS-strspn-c.c += -msse4
endif
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wcscmp-sse2 wcscmp-c wcslen-sse2 wcslen-c \
		   wmemcmp-sse4 wmemcmp-ssse3 wmemcmp-c wcschr-sse2 \
		   wcschr-c wcsrchr-sse2 wcsrchr-c wcscpy-ssse3 wcscpy-c
endif
