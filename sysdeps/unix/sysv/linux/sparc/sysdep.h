/* Copyright (C) 2000-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2000.

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
   <https://www.gnu.org/licenses/>.  */

#ifndef _LINUX_SPARC_SYSDEP_H
#define _LINUX_SPARC_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/sparc/sysdep.h>

#ifdef __ASSEMBLER__

#define	ret		retl; nop
#define	ret_NOERRNO	retl; nop
#define	ret_ERRVAL	retl; nop
#define	r0		%o0
#define	r1		%o1
#define	MOVE(x,y)	mov x, y

#else	/* __ASSEMBLER__ */

# define VDSO_NAME  "LINUX_2.6"
# define VDSO_HASH  61765110

/* List of system calls which are supported as vsyscalls.  */
# ifdef __arch64__
#  define HAVE_CLOCK_GETTIME64_VSYSCALL	"__vdso_clock_gettime"
# else
#  define HAVE_CLOCK_GETTIME_VSYSCALL	"__vdso_clock_gettime"
# endif
# define HAVE_GETTIMEOFDAY_VSYSCALL	"__vdso_gettimeofday"

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) 				\
({	INTERNAL_SYSCALL_DECL(err);  					\
	unsigned long resultvar = INTERNAL_SYSCALL(name, err, nr, args);\
	if (INTERNAL_SYSCALL_ERROR_P (resultvar, err))			\
	  {		     			       		   	\
	    __set_errno (INTERNAL_SYSCALL_ERRNO (resultvar, err));	\
	    resultvar = (unsigned long) -1;				\
	  } 	      							\
	(long) resultvar;						\
})

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...) \
  internal_syscall##nr(__SYSCALL_STRING, err, __NR_##name, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
  internal_syscall##nr(__SYSCALL_STRING, err, name, args)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned long int) (val) > -4096UL)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#define internal_syscall0(string,err,name,dummy...)			\
({									\
	register long int __g1 __asm__ ("g1") = (name);			\
	register long __o0 __asm__ ("o0");				\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall1(string,err,name,arg1)				\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0) :			\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall2(string,err,name,arg1,arg2)			\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0), "r" (__o1) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall3(string,err,name,arg1,arg2,arg3)		\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0), "r" (__o1),		\
			  "r" (__o2) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall4(string,err,name,arg1,arg2,arg3,arg4)		\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3) :			\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall5(string,err,name,arg1,arg2,arg3,arg4,arg5)	\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define internal_syscall6(string,err,name,arg1,arg2,arg3,arg4,arg5,arg6)\
({									\
	register long int __g1 __asm__("g1") = (name);			\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __o5 __asm__ ("o5") = (long)(arg6);		\
	__asm __volatile (string : "=r" (__o0) :			\
			  "r" (__g1), "0" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4),		\
			  "r" (__o5) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define INLINE_CLONE_SYSCALL(arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_clone;			\
	__asm __volatile (__SYSCALL_STRING :				\
			  "=r" (__o0), "=r" (__o1) :			\
			  "r" (__g1), "0" (__o0), "1" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4) :		\
			  __SYSCALL_CLOBBERS);				\
	if (__glibc_unlikely ((unsigned long int) (__o0) > -4096UL))	\
	  {		     			       		   	\
	    __set_errno (-__o0);					\
	    __o0 = -1L;			    				\
	  } 	      							\
	else								\
	  { 	      							\
	    __o0 &= (__o1 - 1);						\
	  } 	    	    						\
	__o0;								\
})

#endif	/* __ASSEMBLER__ */

#endif /* _LINUX_SPARC_SYSDEP_H */
