/* Assembler macros for ARC.
   Copyright (C) 2000-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_ARC_SYSDEP_H
#define _LINUX_ARC_SYSDEP_H 1

#include <asm/unistd.h>
#include <sysdeps/arc/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>

/* For RTLD_PRIVATE_ERRNO.  */
#include <dl-sysdep.h>

#include <tls.h>

#undef SYS_ify
#define SYS_ify(syscall_name)   __NR_##syscall_name

#ifdef __ASSEMBLER__

/* This is a "normal" system call stub: if there is an error,
   it returns -1 and sets errno.  */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)			\
  PSEUDO_NOERRNO(name, syscall_name, args)	ASM_LINE_SEP	\
    brhi   r0, -1024, .Lcall_syscall_err	ASM_LINE_SEP

# define ret	j  [blink]

# undef PSEUDO_END
# define PSEUDO_END(name)					\
  SYSCALL_ERROR_HANDLER				ASM_LINE_SEP	\
  END (name)

/* --------- Helper for SYSCALL_NOERRNO -----------
   This kind of system call stub never returns an error.
   We return the return value register to the caller unexamined.  */

# undef PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)		\
  .text						ASM_LINE_SEP	\
  ENTRY (name)					ASM_LINE_SEP	\
    DO_CALL (syscall_name, args)		ASM_LINE_SEP	\

/* Return the return value register unexamined. Since r0 is both
   syscall return reg and function return reg, no work needed.  */
# define ret_NOERRNO						\
  j_s  [blink]		ASM_LINE_SEP

# undef PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)				\
  END (name)

/* --------- Helper for SYSCALL_ERRVAL -----------
   This kind of system call stub returns the errno code as its return
   value, or zero for success.  We may massage the kernel's return value
   to meet that ABI, but we never set errno here.  */

# undef PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args)		\
  PSEUDO_NOERRNO(name, syscall_name, args)	ASM_LINE_SEP

/* Don't set errno, return kernel error (in errno form) or zero.  */
# define ret_ERRVAL						\
  rsub   r0, r0, 0				ASM_LINE_SEP	\
  ret_NOERRNO

# undef PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name)				\
  END (name)


/* To reduce the code footprint, we confine the actual errno access
   to single place in __syscall_error().
   This takes raw kernel error value, sets errno and returns -1.  */
# if IS_IN (libc)
#  define CALL_ERRNO_SETTER_C	bl     PLTJMP(HIDDEN_JUMPTARGET(__syscall_error))
# else
#  define CALL_ERRNO_SETTER_C	bl     PLTJMP(__syscall_error)
# endif

# define SYSCALL_ERROR_HANDLER					\
.Lcall_syscall_err:				ASM_LINE_SEP	\
    st.a   blink, [sp, -4]			ASM_LINE_SEP	\
    cfi_adjust_cfa_offset (4)			ASM_LINE_SEP	\
    cfi_rel_offset (blink, 0)			ASM_LINE_SEP	\
    CALL_ERRNO_SETTER_C				ASM_LINE_SEP	\
    ld.ab  blink, [sp, 4]			ASM_LINE_SEP	\
    cfi_adjust_cfa_offset (-4)			ASM_LINE_SEP	\
    cfi_restore (blink)				ASM_LINE_SEP	\
    j      [blink]

# define DO_CALL(syscall_name, args)				\
    mov    r8, SYS_ify (syscall_name)		ASM_LINE_SEP	\
    ARC_TRAP_INSN				ASM_LINE_SEP

# define ARC_TRAP_INSN	trap_s 0

#else  /* !__ASSEMBLER__ */

# define SINGLE_THREAD_BY_GLOBAL		1

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#include <errno.h>

extern int __syscall_error (int);

# if IS_IN (libc)
hidden_proto (__syscall_error)
#  define CALL_ERRNO_SETTER   "bl   __syscall_error    \n\t"
# else
#  define CALL_ERRNO_SETTER   "bl   __syscall_error@plt    \n\t"
# endif


/* Define a macro which expands into the inline wrapper code for a system
   call.  */
# undef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr_args, args...)				\
  ({									\
    register int __res __asm__("r0");					\
    __res = INTERNAL_SYSCALL_NCS (__NR_##name, , nr_args, args);	\
    if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P ((__res), ), 0))	\
      {									\
        asm volatile ("st.a blink, [sp, -4] \n\t"			\
                      CALL_ERRNO_SETTER					\
                      "ld.ab blink, [sp, 4] \n\t"			\
                      :"+r" (__res)					\
                      :							\
                      :"r1","r2","r3","r4","r5","r6",			\
                       "r7","r8","r9","r10","r11","r12");		\
       }								\
     __res;								\
 })

# undef INTERNAL_SYSCALL_DECL
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)

# undef INTERNAL_SYSCALL_ERRNO
# define INTERNAL_SYSCALL_ERRNO(val, err)    (-(val))

/* -1 to -1023 are valid errno values.  */
# undef INTERNAL_SYSCALL_ERROR_P
# define INTERNAL_SYSCALL_ERROR_P(val, err)	\
	((unsigned int) (val) > -1024U)

# define ARC_TRAP_INSN	"trap_s 0	\n\t"

# undef INTERNAL_SYSCALL_RAW
# define INTERNAL_SYSCALL_RAW(name, err, nr_args, args...)	\
  ({								\
    /* Per ABI, r0 is 1st arg and return reg.  */		\
    register int __ret __asm__("r0");				\
    register int _sys_num __asm__("r8");			\
								\
    LOAD_ARGS_##nr_args (name, args)				\
								\
    __asm__ volatile (						\
                      ARC_TRAP_INSN				\
                      : "+r" (__ret)				\
                      : "r"(_sys_num) ASM_ARGS_##nr_args	\
                      : "memory");				\
__ret;								\
})

/* Macros for setting up inline __asm__ input regs.  */
# define ASM_ARGS_0
# define ASM_ARGS_1	ASM_ARGS_0, "r" (__ret)
# define ASM_ARGS_2	ASM_ARGS_1, "r" (_arg2)
# define ASM_ARGS_3	ASM_ARGS_2, "r" (_arg3)
# define ASM_ARGS_4	ASM_ARGS_3, "r" (_arg4)
# define ASM_ARGS_5	ASM_ARGS_4, "r" (_arg5)
# define ASM_ARGS_6	ASM_ARGS_5, "r" (_arg6)
# define ASM_ARGS_7	ASM_ARGS_6, "r" (_arg7)

/* Macros for converting sys-call wrapper args into sys call args.  */
# define LOAD_ARGS_0(nm, arg)				\
  _sys_num = (int) (nm);

# define LOAD_ARGS_1(nm, arg1)				\
  __ret = (int) (arg1);					\
  LOAD_ARGS_0 (nm, arg1)

/* Note that the use of _tmpX might look superflous, however it is needed
   to ensure that register variables are not clobbered if arg happens to be
   a function call itself. e.g. sched_setaffinity() calling getpid() for arg2
   Also this specific order of recursive calling is important to segregate
   the tmp args evaluation (function call case described above) and assigment
   of register variables.  */

# define LOAD_ARGS_2(nm, arg1, arg2)			\
  int _tmp2 = (int) (arg2);				\
  LOAD_ARGS_1 (nm, arg1)				\
  register int _arg2 __asm__ ("r1") = _tmp2;

# define LOAD_ARGS_3(nm, arg1, arg2, arg3)		\
  int _tmp3 = (int) (arg3);				\
  LOAD_ARGS_2 (nm, arg1, arg2)				\
  register int _arg3 __asm__ ("r2") = _tmp3;

#define LOAD_ARGS_4(nm, arg1, arg2, arg3, arg4)		\
  int _tmp4 = (int) (arg4);				\
  LOAD_ARGS_3 (nm, arg1, arg2, arg3)			\
  register int _arg4 __asm__ ("r3") = _tmp4;

# define LOAD_ARGS_5(nm, arg1, arg2, arg3, arg4, arg5)	\
  int _tmp5 = (int) (arg5);				\
  LOAD_ARGS_4 (nm, arg1, arg2, arg3, arg4)		\
  register int _arg5 __asm__ ("r4") = _tmp5;

# define LOAD_ARGS_6(nm,  arg1, arg2, arg3, arg4, arg5, arg6)\
  int _tmp6 = (int) (arg6);				\
  LOAD_ARGS_5 (nm, arg1, arg2, arg3, arg4, arg5)	\
  register int _arg6 __asm__ ("r5") = _tmp6;

# define LOAD_ARGS_7(nm, arg1, arg2, arg3, arg4, arg5, arg6, arg7)\
  int _tmp7 = (int) (arg7);				\
  LOAD_ARGS_6 (nm, arg1, arg2, arg3, arg4, arg5, arg6)	\
  register int _arg7 __asm__ ("r6") = _tmp7;

# undef INTERNAL_SYSCALL
# define INTERNAL_SYSCALL(name, err, nr, args...) 	\
  INTERNAL_SYSCALL_RAW(SYS_ify(name), err, nr, args)

# undef INTERNAL_SYSCALL_NCS
# define INTERNAL_SYSCALL_NCS(number, err, nr, args...) \
  INTERNAL_SYSCALL_RAW(number, err, nr, args)

/* Pointer mangling not yet supported.  */
# define PTR_MANGLE(var) (void) (var)
# define PTR_DEMANGLE(var) (void) (var)

#endif /* !__ASSEMBLER__ */

#endif /* linux/arc/sysdep.h */
