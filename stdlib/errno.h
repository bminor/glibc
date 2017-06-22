/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
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

/*
 *	ISO C99 Standard: 7.5 Errors	<errno.h>
 */

#ifndef	_ERRNO_H
#define	_ERRNO_H 1

#include <features.h>

/* The system-specific definitions of the E* constants, as macros.  */
#include <bits/errno.h>

/* When included from assembly language, this header only provides the
   E* constants.  */
#ifndef __ASSEMBLER__

__BEGIN_DECLS

/* This type cues the GDB pretty-printer for errno (errno-printer.py)
   to show a symbolic name for the error.  */
typedef int __error_t;

/* The error code set by various library functions.  */
extern __error_t *__errno_location (void) __THROW __attribute_const__;
# define errno (*__errno_location ())

# ifdef __USE_GNU

/* The full and simple forms of the name with which the program was
   invoked.  These variables are set up automatically at startup based on
   the value of argv[0].  */
extern char *program_invocation_name;
extern char *program_invocation_short_name;

/* User namespace version of __error_t.  This is used to make the return
   values of certain GNU extension functions more self-documenting.  */
#  ifndef __error_t_defined
#   define __error_t_defined 1
typedef __error_t error_t;
#  endif

# endif /* __USE_GNU */

__END_DECLS

#endif /* !__ASSEMBLER__ */
#endif /* errno.h */
