/* Copyright (C) 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* sysdeps/mach/MACHINE/sysdep.h should define these macros.  */

/* Produce a text assembler label for the C global symbol NAME.  */
#ifndef ENTRY
#define ENTRY(name)
#error ENTRY not defined by sysdeps/mach/MACHINE/sysdep.h
#endif

/* Define a C function called NAME which does system call NUMBER.  */
#ifndef SYSCALL_TRAP
#define SYSCALL_TRAP(name, number)
#error SYSCALL_TRAP not defined by sysdeps/mach/MACHINE/sysdep.h
#endif

/* Set variables ARGC, ARGV, and ENVP for the arguments
   left on the stack by the microkernel.  */
#ifndef SNARF_ARGS
#define SNARF_ARGS(argc, argv, envp)
#error SNARF_ARGS not defined by sysdeps/mach/MACHINE/sysdep.h
#endif

/* Call the C function FN with no arguments,
   on a stack starting at SP (as returned by *_cthread_init_routine).
   You don't need to deal with FN returning; it shouldn't.  */
#ifndef	CALL_WITH_SP
#define CALL_WITH_SP(fn, sp)
#error CALL_WITH_SP not defined by sysdeps/mach/MACHINE/sysdep.h
#endif

/* LOSE can be defined as the `halt' instruction or something
   similar which will cause the process to die in a characteristic
   way suggesting a bug.  */
#ifndef LOSE
#define	LOSE	__task_terminate (__mach_task_self ())
#endif

