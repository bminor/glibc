ifeq ($(subdir),rt)
librt-sysdep_routines += timer_routines
CPPFLAGS += -DBROKEN_THREAD_SIGNALS

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librt.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librt.a $(static-thread-library)
endif
endif

ifeq ($(subdir),posix)
CFLAGS-confstr.c += -DLIBPTHREAD_VERSION="\"$(shell sed 's/\(.*\) by .*/\1/' ../linuxthreads/Banner)\""
endif

ifeq ($(subdir),csu)
# Find out the size of NPTL struct pthread
ifneq (,$(wildcard $(..)nptl/descr.h))
gen-as-const-headers += nptl-struct-pthread.sym
else
before-compile += $(common-objpfx)nptl-struct-pthread.h
common-generated += nptl-struct-pthread.h
$(common-objpfx)nptl-struct-pthread.h:
	@echo '#define NPTL_STRUCT_PTHREAD_SIZE 0' > $@
endif
endif
