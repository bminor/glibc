/* Macros for defining Systemtap <sys/sdt.h> static probe points.
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _STAP_PROBE_H
#define _STAP_PROBE_H 1

#ifdef USE_STAP_PROBE

# include <sys/sdt.h>

/* Our code uses one macro LIBC_PROBE (name, n, arg1, ..., argn).

   Without USE_STAP_PROBE, that does nothing but evaluates all
   its arguments (to prevent bit rot, unlike e.g. assert).

   Systemtap's header defines the macros STAP_PROBE (provider, name) and
   STAP_PROBEn (provider, name, arg1, ..., argn).  For "provider" we paste
   in the IN_LIB name (libc, libpthread, etc.) automagically.  */

# ifndef NOT_IN_libc
#  define IN_LIB	libc
# elif !defined IN_LIB
/* This is intentionally defined with extra unquoted commas in it so
   that macro substitution will bomb out when it is used.  We don't
   just use #error here, so that this header can be included by
   other headers that use LIBC_PROBE inside their own macros.  We
   only want such headers to fail to compile if those macros are
   actually used in a context where IN_LIB has not been defined.  */
#  define IN_LIB	,,,missing -DIN_LIB=... -- not extra-lib.mk?,,,
# endif

# define LIBC_PROBE(name, n, ...)	\
  LIBC_PROBE_1 (IN_LIB, name, n, ## __VA_ARGS__)

# define LIBC_PROBE_1(lib, name, n, ...) \
  STAP_PROBE##n (lib, name, ## __VA_ARGS__)

# define STAP_PROBE0		STAP_PROBE

# define LIBC_PROBE_ASM(name, template) \
  STAP_PROBE_ASM (IN_LIB, name, template)

# define LIBC_PROBE_ASM_OPERANDS STAP_PROBE_ASM_OPERANDS

#else  /* Not USE_STAP_PROBE.  */

# ifndef __ASSEMBLER__
#  define LIBC_PROBE(name, n, ...)		DUMMY_PROBE##n (__VA_ARGS__)
# else
#  define LIBC_PROBE(name, n, ...)		/* Nothing.  */
# endif

# define LIBC_PROBE_ASM(name, template)		/* Nothing.  */
# define LIBC_PROBE_ASM_OPERANDS(n, ...)	/* Nothing.  */

/* This silliness lets us evaluate all the arguments for each arity
   of probe.  My kingdom for a real macro system.  */

# define DUMMY_PROBE0()			do {} while (0)
# define DUMMY_PROBE1(a1)		do {} while ((void) (a1), 0)
# define DUMMY_PROBE2(a1, a2)		do {} while ((void) (a1), \
						     (void) (a2), 0)
# define DUMMY_PROBE3(a1, a2, a3)	do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), 0)
# define DUMMY_PROBE4(a1, a2, a3, a4)	do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), 0)
# define DUMMY_PROBE5(a1, a2, a3, a4, a5)			  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), 0)
# define DUMMY_PROBE6(a1, a2, a3, a4, a5, a6)			  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), \
						     (void) (a6), 0)
# define DUMMY_PROBE7(a1, a2, a3, a4, a5, a6, a7)		  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), \
						     (void) (a6), \
						     (void) (a7), 0)
# define DUMMY_PROBE8(a1, a2, a3, a4, a5, a6, a7, a8)		  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), \
						     (void) (a6), \
						     (void) (a7), \
						     (void) (a8), 0)
# define DUMMY_PROBE9(a1, a2, a3, a4, a5, a6, a7, a8, a9)	  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), \
						     (void) (a6), \
						     (void) (a7), \
						     (void) (a8), \
						     (void) (a9), 0)
# define DUMMY_PROBE10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)	  \
					do {} while ((void) (a1), \
						     (void) (a2), \
						     (void) (a3), \
						     (void) (a4), \
						     (void) (a5), \
						     (void) (a6), \
						     (void) (a7), \
						     (void) (a8), \
						     (void) (a9), \
						     (void) (a10), 0)

#endif	/* USE_STAP_PROBE.  */

#endif	/* stap-probe.h */
