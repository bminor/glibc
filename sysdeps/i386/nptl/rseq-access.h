/* RSEQ_* accessors.  i386 version.
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

#define __RSEQ_GETMEM(member) \
  ({ __typeof (RSEQ_SELF()->member) __value;				      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%gs:%P2(%3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct rseq_area, member)),   \
		     "r" (__rseq_offset));				      \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%gs:%P1(%2),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct rseq_area, member)),	      \
		       "r" (__rseq_offset));				      \
     else /* 8 */							      \
       {								      \
	 asm volatile  ("movl %%gs:%P1(%2),%%eax\n\t"			      \
			"movl %%gs:4+%P1(%2),%%edx"			      \
			: "=&A" (__value)				      \
			: "i" (offsetof (struct rseq_area, member)),	      \
			  "r" (__rseq_offset));				      \
       }								      \
     __value; })

/* Read member of the RSEQ area directly.  */
#define RSEQ_GETMEM(member) \
  ({									      \
     _Static_assert (sizeof (RSEQ_SELF()->member) == 1			      \
		     || sizeof (RSEQ_SELF()->member) == 4		      \
		     || sizeof (RSEQ_SELF()->member) == 8,		      \
		     "size of rseq data");				      \
     __RSEQ_GETMEM(member); })

/* Read member of the RSEQ area directly, with single-copy atomicity semantics.
   Static assert for types >= 64 bits since they can't be loaded atomically on
   x86-32.  */
#define RSEQ_GETMEM_ONCE(member) \
  ({									      \
     _Static_assert (sizeof (RSEQ_SELF()->member) == 1			      \
		     || sizeof (RSEQ_SELF()->member) == 4,		      \
		     "size of rseq data");				      \
     __RSEQ_GETMEM(member); })

#define __RSEQ_SETMEM(member, value) \
  ({									      \
     if (sizeof (RSEQ_SELF()->member) == 1)				      \
       asm volatile ("movb %b0,%%gs:%P1(%2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct rseq_area, member)),	      \
		       "r" (__rseq_offset));				      \
     else if (sizeof (RSEQ_SELF()->member) == 4)			      \
       asm volatile ("movl %0,%%gs:%P1(%2)" :				      \
		     : "ir" (value),					      \
		       "i" (offsetof (struct rseq_area, member)),	      \
		       "r" (__rseq_offset));				      \
     else /* 8 */							      \
       {								      \
	 asm volatile ("movl %%eax,%%gs:%P1(%2)\n\t"			      \
		       "movl %%edx,%%gs:4+%P1(%2)" :			      \
		       : "A" ((uint64_t) cast_to_integer (value)),	      \
			 "i" (offsetof (struct rseq_area, member)),	      \
			 "r" (__rseq_offset));				      \
       }})

/* Set member of the RSEQ area directly.  */
#define RSEQ_SETMEM(member, value) \
  ({									      \
     _Static_assert (sizeof (RSEQ_SELF()->member) == 1			      \
		     || sizeof (RSEQ_SELF()->member) == 4		      \
		     || sizeof (RSEQ_SELF()->member) == 8,		      \
		     "size of rseq data");				      \
     __RSEQ_SETMEM(member, value); })

/* Set member of the RSEQ area directly, with single-copy atomicity semantics.
   Static assert for types >= 64 bits since they can't be stored atomically on
   x86-32.  */
#define RSEQ_SETMEM_ONCE(member, value) \
  ({									      \
     _Static_assert (sizeof (RSEQ_SELF()->member) == 1			      \
		     || sizeof (RSEQ_SELF()->member) == 4,		      \
		     "size of rseq data");				      \
     __RSEQ_SETMEM(member, value); })
