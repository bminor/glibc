ifeq ($(subdir),csu)
aux += init-arch
tests += test-multiarch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)

sysdep_routines += strncat-c stpncpy-c strncpy-c strcmp-ssse3 \
		   strcmp-sse2-unaligned strncmp-sse2-unaligned strncmp-ssse3 \
		   memcpy-ssse3 \
		   memcpy-sse2-unaligned mempcpy-ssse3 \
		   memmove-ssse3 memcpy-ssse3-back mempcpy-ssse3-back \
		   memmove-avx-unaligned memcpy-avx-unaligned mempcpy-avx-unaligned \
		   memmove-ssse3-back strcasecmp_l-ssse3 \
		   strcasecmp_l-sse2-unaligned strncase_l-sse2-unaligned \
		   strncase_l-ssse3 strcat-ssse3 strncat-ssse3\
		   strcpy-ssse3 strncpy-ssse3 stpcpy-ssse3 stpncpy-ssse3 \
		   strcpy-sse2-unaligned strncpy-sse2-unaligned \
		   stpcpy-sse2-unaligned stpncpy-sse2-unaligned \
		   strcat-sse2-unaligned strncat-sse2-unaligned \
		   strchr-sse2-no-bsf memcmp-ssse3 strstr-sse2-unaligned \
		   strcspn_sse2 strpbrk_sse2 strcspn_sse2_late strpbrk_sse2_late

ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn_sse42 strpbrk_sse42 strspn_sse42
endif

ifeq (yes,$(config-cflags-avx2))
sysdep_routines += memset-avx2 strcpy-avx2 stpcpy-avx2 memcmp-avx2  strcmp-avx2 strncmp-avx2 strcasecmp_l-avx2 strncase_l-avx2
endif
endif

ifeq ($(subdir),wcsmbs)
sysdep_routines += wmemcmp-sse2-unaligned wmemcmp-ssse3 wmemcmp-c wcscpy-ssse3 wcscpy-c
endif
