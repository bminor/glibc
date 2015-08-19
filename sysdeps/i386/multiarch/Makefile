ifeq ($(subdir),csu)
tests += test-multiarch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
gen-as-const-headers += locale-defines.sym
sysdep_routines += bzero-i386 bzero-i586 bzero-i686 \
		   bzero-sse2 bzero-sse2-rep \
		   memset-i386 memset-i586 memset-i686 \
		   memset-sse2 memset-sse2-rep
endif
