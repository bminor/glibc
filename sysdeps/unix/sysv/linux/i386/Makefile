# The default ABI is 32.
default-abi := 32

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86
# %ebp may be used to pass the 6th argument to syscall.
CFLAGS-epoll_pwait.o += -fomit-frame-pointer
CFLAGS-epoll_pwait.os += -fomit-frame-pointer
CFLAGS-mmap.o += -fomit-frame-pointer
CFLAGS-mmap.os += -fomit-frame-pointer
CFLAGS-mmap64.o += -fomit-frame-pointer
CFLAGS-mmap64.os += -fomit-frame-pointer
CFLAGS-pselect.o += -fomit-frame-pointer
CFLAGS-pselect.os += -fomit-frame-pointer
CFLAGS-rtld-mmap.o += -fomit-frame-pointer
CFLAGS-rtld-mmap.os += -fomit-frame-pointer
endif

ifeq ($(subdir),sysvipc)
# %ebp may be used to pass the 6th argument to syscall.
CFLAGS-semtimedop.o += -fomit-frame-pointer
CFLAGS-semtimedop.os += -fomit-frame-pointer
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

# fallocate, posix_fallocate use six-argument inline syscalls.
ifeq ($(subdir),io)
sysdep_routines += libc-do-syscall
# %ebp may be used to pass the 6th argument to syscall.
CFLAGS-fallocate.o += -fomit-frame-pointer
CFLAGS-fallocate.os += -fomit-frame-pointer
CFLAGS-fallocate64.o += -fomit-frame-pointer
CFLAGS-fallocate64.os += -fomit-frame-pointer
CFLAGS-posix_fallocate.o += -fomit-frame-pointer
CFLAGS-posix_fallocate.os += -fomit-frame-pointer
CFLAGS-posix_fallocate64.o += -fomit-frame-pointer
CFLAGS-posix_fallocate64.os += -fomit-frame-pointer
CFLAGS-sync_file_range.o += -fomit-frame-pointer
CFLAGS-sync_file_range.os += -fomit-frame-pointer
endif

# libpthread uses six-argument inline syscalls.
ifeq ($(subdir),nptl)
libpthread-sysdep_routines += libc-do-syscall
# %ebp may be used to pass the 6th argument to syscall.
CFLAGS-pthread_rwlock_timedrdlock.o += -fomit-frame-pointer
CFLAGS-pthread_rwlock_timedrdlock.os += -fomit-frame-pointer
CFLAGS-pthread_rwlock_timedwrlock.o += -fomit-frame-pointer
CFLAGS-pthread_rwlock_timedwrlock.os += -fomit-frame-pointer
CFLAGS-sem_timedwait.o += -fomit-frame-pointer
CFLAGS-sem_timedwait.os += -fomit-frame-pointer
CFLAGS-sem_wait.o += -fomit-frame-pointer
CFLAGS-sem_wait.os += -fomit-frame-pointer
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),csu)
sysdep-dl-routines += sysdep
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
endif

ifeq ($(subdir),rt)
# pull in __syscall_error routine
librt-routines += sysdep
endif
