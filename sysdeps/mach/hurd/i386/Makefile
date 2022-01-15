ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/io.h
endif

ifeq ($(subdir),debug)
gen-as-const-headers += signal-defines.sym
endif

ifeq ($(subdir),setjmp)
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
# (type conformance)
conformtest-xfail-conds += i386-gnu
endif

# For bug 23286
ifeq ($(subdir),hurd)
test-xfail-check-abi-libhurduser = yes
endif
ifeq ($(subdir),mach)
test-xfail-check-abi-libmachuser = yes
endif

# For bug 25521
# (setpshared support)
ifeq ($(subdir),malloc)
test-xfail-tst-mallocfork2 = yes
test-xfail-tst-mallocfork2-mcheck = yes
test-xfail-tst-mallocfork2-malloc-check = yes
test-xfail-tst-mallocfork2-malloc-hugetlb1 = yes
test-xfail-tst-mallocfork2-malloc-hugetlb2 = yes
endif
ifeq ($(subdir),htl)
test-xfail-tst-mutex4 = yes
test-xfail-tst-cancel16 = yes
test-xfail-tst-cancelx16 = yes
test-xfail-tst-cond4 = yes
test-xfail-tst-cond6 = yes
test-xfail-tst-cond12 = yes
test-xfail-tst-cond13 = yes
test-xfail-tst-cond23 = yes
test-xfail-tst-rwlock4 = yes
test-xfail-tst-rwlock12 = yes
test-xfail-tst-barrier2 = yes
test-xfail-tst-pututxline-cache = yes
test-xfail-tst-pututxline-lockfail = yes
test-xfail-tst-flock2 = yes
test-xfail-tst-signal1 = yes
test-xfail-tst-signal2 = yes
endif

ifeq ($(subdir),htl)
# For bug 25522
# (setprotocol support)
test-xfail-tst-cond24 = yes
test-xfail-tst-cond25 = yes

# For bug 25563
# (robust support against dead threads)
test-xfail-tst-robust1 = yes
test-xfail-tst-robust2 = yes
test-xfail-tst-robust3 = yes
test-xfail-tst-robust4 = yes
test-xfail-tst-robust5 = yes
test-xfail-tst-robust6 = yes
test-xfail-tst-robust7 = yes
test-xfail-tst-robust9 = yes
endif

ifeq ($(subdir),elf)
# We do use nested functions involving creation of trampolines, notably for
# callbacks whose parameters don't permit to get the context parameters.
check-execstack-xfail += ld.so libc.so libpthread.so
# We always create a thread for signals
test-xfail-tst-single_threaded-pthread-static = yes
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif
