ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/io.h
endif

ifeq ($(subdir),debug)
gen-as-const-headers += signal-defines.sym
endif

ifeq ($(subdir),csu)
ifeq (yes,$(build-shared))
sysdep_routines += divdi3
shared-only-routines += divdi3
CPPFLAGS-divdi3.c = -Din_divdi3_c
endif
endif

ifeq ($(subdir),conform)
# For bugs 23081, 23082, 23083, 23084, 23085, 23086.
conformtest-xfail-conds += i386-gnu

# For bug 23088
test-xfail-POSIX/signal.h/conform = yes
test-xfail-POSIX/regex.h/conform = yes
test-xfail-POSIX/aio.h/conform = yes
test-xfail-POSIX/mqueue.h/conform = yes
test-xfail-POSIX/sys/types.h/conform = yes
test-xfail-UNIX98/signal.h/conform = yes
test-xfail-UNIX98/regex.h/conform = yes
test-xfail-UNIX98/aio.h/conform = yes
test-xfail-UNIX98/mqueue.h/conform = yes
test-xfail-UNIX98/sys/types.h/conform = yes
test-xfail-UNIX98/stdlib.h/conform = yes
test-xfail-POSIX2008/signal.h/conform = yes
test-xfail-POSIX2008/regex.h/conform = yes
test-xfail-POSIX2008/aio.h/conform = yes
test-xfail-POSIX2008/mqueue.h/conform = yes
test-xfail-POSIX2008/sys/types.h/conform = yes
test-xfail-XOPEN2K/signal.h/conform = yes
test-xfail-XOPEN2K/regex.h/conform = yes
test-xfail-XOPEN2K/aio.h/conform = yes
test-xfail-XOPEN2K/mqueue.h/conform = yes
test-xfail-XOPEN2K/sys/types.h/conform = yes
test-xfail-XOPEN2K/stdlib.h/conform = yes
test-xfail-XOPEN2K8/signal.h/conform = yes
test-xfail-XOPEN2K8/regex.h/conform = yes
test-xfail-XOPEN2K8/aio.h/conform = yes
test-xfail-XOPEN2K8/mqueue.h/conform = yes
test-xfail-XOPEN2K8/sys/types.h/conform = yes
test-xfail-XOPEN2K8/stdlib.h/conform = yes

# For bug 23819
test-xfail-ISO11/threads.h/linknamespace = yes
test-xfail-ISO11/threads.h/conform = yes
endif

# For bug 23286
ifeq ($(subdir),hurd)
test-xfail-check-abi-libhurduser = yes
endif
ifeq ($(subdir),mach)
test-xfail-check-abi-libmachuser = yes
endif

ifeq ($(subdir),elf)
# We do use nested functions involving creation of trampolines, notably for
# callbacks whose parameters don't permit to get the context parameters.
check-execstack-xfail += ld.so libc.so libpthread.so
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
