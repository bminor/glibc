/* RSEQ_* accessors.  x86_64 version.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

/* Read member of the RSEQ area directly, with single-copy atomicity semantics.  */
#define RSEQ_GETMEM_ONCE(member) \
  ({ __typeof (RSEQ_SELF()->member) __value;				      \
     _Static_assert (sizeof (__value) == 1				      \
		     || sizeof (__value) == 4				      \
		     || sizeof (__value) == 8,				      \
		     "size of rseq data");			      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%fs:%P2(%q3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct rseq_area, member)),    \
		       "r" ((long long int) __rseq_offset));		      \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%fs:%P1(%q2),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct rseq_area, member)),	      \
		       "r" ((long long int) __rseq_offset));		      \
     else /* 8 */							      \
       {								      \
	 asm volatile ("movq %%fs:%P1(%q2),%q0"				      \
		       : "=r" (__value)					      \
		       : "i" (offsetof (struct rseq_area, member)),	      \
		         "r" ((long long int) __rseq_offset));		      \
       }								      \
     __value; })

/* Read member of the RSEQ area directly.  */
#define RSEQ_GETMEM(member) RSEQ_GETMEM_ONCE(member)

/* Set member of the RSEQ area directly, with single-copy atomicity semantics.  */
#define RSEQ_SETMEM_ONCE(member, value) \
  ({									      \
     _Static_assert (sizeof (RSEQ_SELF()->member) == 1			      \
		     || sizeof (RSEQ_SELF()->member) == 4		      \
		     || sizeof (RSEQ_SELF()->member) == 8,		      \
		     "size of rseq data");			      \
     if (sizeof (RSEQ_SELF()->member) == 1)				      \
       asm volatile ("movb %b0,%%fs:%P1(%q2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct rseq_area, member)),	      \
		       "r" ((long long int) __rseq_offset));		      \
     else if (sizeof (RSEQ_SELF()->member) == 4)			      \
       asm volatile ("movl %0,%%fs:%P1(%q2)" :				      \
		     : IMM_MODE (value),				      \
		       "i" (offsetof (struct rseq_area, member)),	      \
		       "r" ((long long int) __rseq_offset));		      \
     else /* 8 */							      \
       {								      \
	 /* Since movq takes a signed 32-bit immediate or a register source   \
	    operand, use "er" constraint for 32-bit signed integer constant   \
	    or register.  */						      \
	 asm volatile ("movq %q0,%%fs:%P1(%q2)" :			      \
		       : "er" ((uint64_t) cast_to_integer (value)),	      \
			 "i" (offsetof (struct rseq_area, member)),	      \
		         "r" ((long long int) __rseq_offset));		      \
       }})

/* Set member of the RSEQ area directly.  */
#define RSEQ_SETMEM(member, value) RSEQ_SETMEM_ONCE(member, value)
