/* Internal per-thread variables for the Hurd.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#ifndef _HURD_PTV_H
#define	_HURD_PTV_H

/* The per-thread variables are found by ANDing this mask
   with the value of the stack pointer and then adding this offset.

   In the multi-threaded case, cthreads initialization sets
   __hurd_ptv_stack_mask to ~(cthread_stack_size - 1), a mask which finds
   the base of the fixed-size cthreads stack; and __hurd_ptv_stack_offset
   to a small offset that skips the data cthreads itself maintains at the
   base of each thread's stack.

   In the single-threaded case, __hurd_ptv_stack_mask is zero, so the stack
   pointer is ignored; and __hurd_ptv_stack_offset gives the address of a
   small allocated region which contains the variables for the single
   thread.  */
   
extern unsigned long int __hurd_ptv_stack_mask;
extern unsigned long int __hurd_ptv_stack_offset;

/* At the location described by the two variables above,
   there are __hurd_ptv_max `unsigned long int's of per-thread data.  */
extern unsigned int __hurd_ptv_max;

/* These values are the indices for the standard per-thread variables.  */
enum __hurd_ptv_index
  {
    _HURD_PTV_ERRNO,		/* `errno' value for this thread.  */
    _HURD_PTV_SIGSTATE,		/* This thread's `struct hurd_sigstate'.  */
    _HURD_PTV_DYNAMIC_USER,	/* Dynamically-assigned user variables. */
    _HURD_PTV_MAX		/* Default value for __hurd_ptv_max.  */
  };


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif

#include <machine-sp.h>		/* Define __thread_stack_pointer.  */

/* Return the location of the current thread's value for the
   per-thread variable with index INDEX.  */

_EXTERN_INLINE unsigned long int *
__hurd_ptv_location (enum __hurd_ptv_index __index)
{
  unsigned long int __stack = (unsigned long int) __thread_stack_pointer ();
  __stack &= __hurd_ptv_stack_mask;
  __stack += __hurd_ptv_stack_offset;
  return &((unsigned long int *) __stack)[__index];
}

/* Return the current thread's location for `errno'.
   The syntax of this function allows redeclarations like `int errno'.  */
_EXTERN_INLINE int *
__hurd_errno_location (void)
{
  return (int *) __hurd_ptv_location (_HURD_PTV_ERRNO);
}

/* XXX to go in errno.h after #include <hurd/ptv.h>: */
#define errno	(*__hurd_errno_location ())


#endif	/* hurd/ptv.h */
