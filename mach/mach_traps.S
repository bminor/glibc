#include <sysdep.h>

#define	SYSCALL(name, number, type, args, typed_args) \
  SYSCALL_TRAP (__##name, number)

#include "mach_syscalls.h"
