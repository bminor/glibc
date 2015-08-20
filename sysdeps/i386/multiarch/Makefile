ifeq ($(subdir),csu)
tests += test-multiarch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
gen-as-const-headers += locale-defines.sym
sysdep_routines += bcopy-i386 bcopy-i686 bcopy-sse2-unaligned \
		   bcopy-ssse3 bcopy-ssse3-rep \
		   memcpy-i386 memcpy-i586 memcpy-i686 \
		   memcpy-sse2-unaligned memcpy-ssse3 memcpy-ssse3-rep \
		   memmove-i386 memmove-i686 memmove-sse2-unaligned \
		   memmove-ssse3 memmove-ssse3-rep \
		   static-memmove \
		   mempcpy-i386 mempcpy-i586 mempcpy-i686 \
		   mempcpy-sse2-unaligned mempcpy-ssse3 mempcpy-ssse3-rep \
		   bzero-i386 bzero-i586 bzero-i686 \
		   bzero-sse2 bzero-sse2-rep \
		   memset-i386 memset-i586 memset-i686 \
		   memset-sse2 memset-sse2-rep \
		   memchr-sse2-bsf memchr-sse2 \
		   memcmp-i386 memcmp-i686 memcmp-ssse3 memcmp-sse4 \
		   memrchr-i386 memrchr-sse2-bsf memrchr-sse2 \
		   rawmemchr-sse2-bsf rawmemchr-sse2 \
		   stpcpy-i386 stpcpy-i586 stpcpy-sse2 stpcpy-ssse3 \
		   stpncpy-i386 stpncpy-sse2 stpncpy-ssse3 \
		   strcpy-i386 strcpy-i586 strcpy-sse2 strcpy-ssse3 \
		   strncpy-i386 strncpy-sse2 strncpy-ssse3 \
		   strcmp-i386 strcmp-i686 strcmp-sse4 strcmp-ssse3 \
		   strcasecmp-i386 strcasecmp_l-i386 strcasecmp_l-sse4 \
		   strcasecmp_l-ssse3 \
		   strncase-i386 strncase_l-i386 strncase_l-sse4 \
		   strncase_l-ssse3 \
		   strncmp-i386 strncmp-ssse3 strncmp-sse4 \
		   strcat-i486 strcat-sse2 strcat-ssse3 \
		   strncat-i386 strncat-sse2 strncat-ssse3
endif

ifeq (mathyes,$(subdir)$(config-cflags-avx))
libm-sysdep_routines += s_fma-fma s_fmaf-fma
CFLAGS-s_fma-fma.c += -mavx -mfpmath=sse
CFLAGS-s_fmaf-fma.c += -mavx -mfpmath=sse
endif
