ifeq ($(subdir),string)
sysdep_routines += varshift \
		   strlen-sse2 strlen-sse2-bsf \
		   strchr-sse2 strrchr-sse2 strchr-sse2-bsf strrchr-sse2-bsf \
		   strnlen-sse2 strnlen-c
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn-c strpbrk-c strspn-c
CFLAGS-varshift.c += -msse4
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
endif
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wcscmp-sse2 wcscmp-c wcslen-sse2 wcslen-c \
		   wmemcmp-sse4 wmemcmp-ssse3 wmemcmp-c wcschr-sse2 \
		   wcschr-c wcsrchr-sse2 wcsrchr-c wcscpy-ssse3 wcscpy-c
endif
