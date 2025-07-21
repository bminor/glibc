/* termios functions internal implementation header for Linux

   Copyright (C) 1991-2025 Free Software Foundation, Inc.
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

#ifndef TERMIOS_INTERNALS_H
#define TERMIOS_INTERNALS_H 1

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sysdep.h>
#include <shlib-compat.h>

#include <termios_arch.h>

/* ---- Kernel interface definitions ---- */

/* The the termios2 structure used in the kernel interfaces is not the
   same as the termios structure we use in the libc.  Therefore we
   must translate it here.  */

struct termios2
{
  tcflag_t c_iflag;		/* input mode flags */
  tcflag_t c_oflag;		/* output mode flags */
  tcflag_t c_cflag;		/* control mode flags */
  tcflag_t c_lflag;		/* local mode flags */
#if _HAVE_TERMIOS2_C_CC_BEFORE_C_LINE
  cc_t c_cc[_TERMIOS2_NCCS];	/* control characters */
  cc_t c_line;			/* line discipline */
#else
  cc_t c_line;			/* line discipline */
  cc_t c_cc[_TERMIOS2_NCCS];	/* control characters */
#endif
  speed_t c_ispeed;		/* input speed */
  speed_t c_ospeed;		/* output speed */
};

/* Alpha got termios2 late, but TCGETS has exactly the same structure
   format and function as TCGETS2. On all other platforms, the termios2
   interface exists as far back as this version of glibc supports.

   For TCSETS* it is more complicated; this is handled in tcsetattr.c.

   Some other architectures only have the equivalent of the termios2
   interface, in which case the old ioctl names are the only ones
   presented, but are equivalent to the new ones. */
#ifndef TCGETS2
# define TCGETS2  TCGETS
# define TCSETS2  TCSETS
# define TCSETSW2 TCSETSW
# define TCSETSF2 TCSETSF
#elif !__ASSUME_TERMIOS2
/* Hack for Alpha */
# undef  TCGETS2
# define TCGETS2 TCGETS
#endif

/* ---- Application interface definitions ---- */

/*
 * Should old speed_t and struct termios (if applicable) compatibility
 * functions be included?
 */
#define _TERMIOS_OLD_COMPAT SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_42)

/*
 * Old struct termios (without c_ispeed and c_ospeed fields) if
 * applicable. The new struct termios *must* be binary identical up to
 * the sizeof the old structure.
 *
 * This only applies to SPARC and MIPS; for other architectures the
 * new and old speed_t interfaces both use the same struct termios.
 */
#if _HAVE_STRUCT_OLD_TERMIOS
typedef struct old_termios old_termios_t;
#else
# define OLD_NCCS NCCS
typedef struct termios old_termios_t;
#endif

/* ---- Internal function definitions ---- */

/*
 * Copy a set of c_cc fields of possibly different width. If the target
 * field is longer, then fill with _POSIX_VDISABLE == -1.
 */
static inline void
copy_c_cc (cc_t *to, size_t nto, const cc_t *from, size_t nfrom)
{
  if (nto < nfrom)
    nfrom = nto;

  to = __mempcpy (to, from, nfrom * sizeof(cc_t));
  if (nto > nfrom)
    memset (to, _POSIX_VDISABLE, (nto - nfrom) * sizeof(cc_t));
}

/* Extract the output and input legacy speed fields from c_cflag. */
static inline tcflag_t
cbaud (tcflag_t c_cflag)
{
  return c_cflag & CBAUD;
}

static inline tcflag_t
cibaud (tcflag_t c_cflag)
{
  return cbaud (c_cflag >> IBSHIFT);
}

extern speed_t
___cbaud_to_speed (tcflag_t c_cflag, speed_t other)
    __attribute_const__ attribute_hidden;

extern tcflag_t
___speed_to_cbaud (speed_t speed)
    __attribute_const__ attribute_hidden;

extern void
___termios2_canonicalize_speeds (struct termios2 *k_termios_p)
    attribute_hidden;

#endif /* TERMIOS_INTERNALS_H */
