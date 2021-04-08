/* Copyright (C) 2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _LINUX_AARCH64_MORELLO_SYSDEP_H
#define _LINUX_AARCH64_MORELLO_SYSDEP_H 1

#include <sysdeps/unix/sysv/linux/aarch64/sysdep.h>

#ifdef __ASSEMBLER__
# if !IS_IN (libc)
#  undef SYSCALL_ERROR_HANDLER
#  if RTLD_PRIVATE_ERRNO
#   define SYSCALL_ERROR_HANDLER				\
.Lsyscall_error:						\
	neg     w0, w0;						\
	adrp    c1, :got:rtld_errno;				\
	ldr     c1, [c1, :got_lo12:rtld_errno];			\
	str     w0, [c1];					\
	mov	x0, -1;						\
	RET;
#  else
#   define SYSCALL_ERROR_HANDLER				\
.Lsyscall_error:						\
	neg	w4, w0;						\
	adrp	c1, :gottprel:errno;				\
	add	c1, c1, :gottprel_lo12:errno;			\
	ldp	x1, x2, [c1];					\
	mrs	c3, ctpidr_el0;					\
	add	c1, c3, x1, uxtx;				\
	scbnds	c1, c1, x2;					\
	mov	x0, -1;						\
	str	w4, [c1];					\
	RET;
#  endif
# endif

#else /* not __ASSEMBLER__ */

typedef __intcap_t __sysarg_t;

# undef INTERNAL_SYSCALL_RAW
# define INTERNAL_SYSCALL_RAW(name, nr, args...)		\
  ({ __sysarg_t _sys_result;						\
     {								\
       LOAD_ARGS_##nr (args)					\
       register long _x8 asm ("x8") = (name);			\
       asm volatile ("svc	0	// syscall " # name     \
		     : "=r" (_x0) : "r"(_x8) ASM_ARGS_##nr : "memory");	\
       _sys_result = _x0;					\
     }								\
     _sys_result; })

# undef LOAD_ARGS_0
# undef LOAD_ARGS_1
# undef LOAD_ARGS_2
# undef LOAD_ARGS_3
# undef LOAD_ARGS_4
# undef LOAD_ARGS_5
# undef LOAD_ARGS_6
# undef LOAD_ARGS_7

# define LOAD_ARGS_0()				\
  register __sysarg_t _x0 asm ("c0");
# define LOAD_ARGS_1(c0)			\
  __sysarg_t _x0tmp = (__sysarg_t) (c0);	\
  LOAD_ARGS_0 ()				\
  _x0 = _x0tmp;
# define LOAD_ARGS_2(c0, c1)			\
  __sysarg_t _x1tmp = (__sysarg_t) (c1);	\
  LOAD_ARGS_1 (c0)				\
  register __sysarg_t _x1 asm ("c1") = _x1tmp;
# define LOAD_ARGS_3(c0, c1, c2)		\
  __sysarg_t _x2tmp = (__sysarg_t) (c2);	\
  LOAD_ARGS_2 (c0, c1)				\
  register __sysarg_t _x2 asm ("c2") = _x2tmp;
# define LOAD_ARGS_4(c0, c1, c2, c3)		\
  __sysarg_t _x3tmp = (__sysarg_t) (c3);	\
  LOAD_ARGS_3 (c0, c1, c2)			\
  register __sysarg_t _x3 asm ("c3") = _x3tmp;
# define LOAD_ARGS_5(c0, c1, c2, c3, c4)	\
  __sysarg_t _x4tmp = (__sysarg_t) (c4);	\
  LOAD_ARGS_4 (c0, c1, c2, c3)			\
  register __sysarg_t _x4 asm ("c4") = _x4tmp;
# define LOAD_ARGS_6(c0, c1, c2, c3, c4, c5)	\
  __sysarg_t _x5tmp = (__sysarg_t) (c5);	\
  LOAD_ARGS_5 (c0, c1, c2, c3, c4)		\
  register __sysarg_t _x5 asm ("c5") = _x5tmp;
# define LOAD_ARGS_7(c0, c1, c2, c3, c4, c5, c6)\
  __sysarg_t _x6tmp = (__sysarg_t) (x6);	\
  LOAD_ARGS_6 (c0, c1, c2, c3, c4, c5)		\
  register __sysarg_t _x6 asm ("c6") = _x6tmp;

#endif	/* __ASSEMBLER__ */

/* Disable pointer mangling for purecap ABI.  */
#undef PTR_MANGLE
#undef PTR_MANGLE2
#undef PTR_DEMANGLE
#undef PTR_DEMANGLE2

#endif /* linux/aarch64/morello/sysdep.h */
