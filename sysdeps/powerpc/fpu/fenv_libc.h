/* Internal libc stuff for floating point environment routines.
   Copyright (C) 1997-2019 Free Software Foundation, Inc.
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

#ifndef _FENV_LIBC_H
#define _FENV_LIBC_H	1

#include <fenv.h>
#include <ldsodefs.h>
#include <sysdep.h>

extern const fenv_t *__fe_nomask_env_priv (void);

extern const fenv_t *__fe_mask_env (void) attribute_hidden;

/* The sticky bits in the FPSCR indicating exceptions have occurred.  */
#define FPSCR_STICKY_BITS ((FE_ALL_EXCEPT | FE_ALL_INVALID) & ~FE_INVALID)

/* Equivalent to fegetenv, but returns a fenv_t instead of taking a
   pointer.  */
#define fegetenv_register() __builtin_mffs()

/* Equivalent to fegetenv_register, but only returns bits for
   status, exception enables, and mode.  */

#define fegetenv_status_ISA300()					\
  ({register double __fr;						\
    __asm__ __volatile__ (						\
      ".machine push; .machine \"power9\"; mffsl %0; .machine pop"	\
      : "=f" (__fr));							\
    __fr;								\
  })

#ifdef _ARCH_PWR9
# define fegetenv_status() fegetenv_status_ISA300()
#elif defined __BUILTIN_CPU_SUPPORTS__
# define fegetenv_status()						\
  (__glibc_likely (__builtin_cpu_supports ("arch_3_00"))		\
   ? fegetenv_status_ISA300()						\
   : fegetenv_register()						\
  )
#else
# define fegetenv_status() fegetenv_register ()
#endif

/* Equivalent to fesetenv, but takes a fenv_t instead of a pointer.  */
#define fesetenv_register(env) \
	do { \
	  double d = (env); \
	  if(GLRO(dl_hwcap) & PPC_FEATURE_HAS_DFP) \
	    asm volatile (".machine push; " \
			  ".machine \"power6\"; " \
			  "mtfsf 0xff,%0,1,0; " \
			  ".machine pop" : : "f" (d)); \
	  else \
	    __builtin_mtfsf (0xff, d); \
	} while(0)

/* This very handy macro:
   - Sets the rounding mode to 'round to nearest';
   - Sets the processor into IEEE mode; and
   - Prevents exceptions from being raised for inexact results.
   These things happen to be exactly what you need for typical elementary
   functions.  */
#define relax_fenv_state() \
	do { \
	   if (GLRO(dl_hwcap) & PPC_FEATURE_HAS_DFP) \
	     asm volatile (".machine push; .machine \"power6\"; " \
		  "mtfsfi 7,0,1; .machine pop"); \
	   asm volatile ("mtfsfi 7,0"); \
	} while(0)

/* Set/clear a particular FPSCR bit (for instance,
   reset_fpscr_bit(FPSCR_VE);
   prevents INVALID exceptions from being raised).  */
#define set_fpscr_bit(x) asm volatile ("mtfsb1 %0" : : "i"(x))
#define reset_fpscr_bit(x) asm volatile ("mtfsb0 %0" : : "i"(x))

typedef union
{
  fenv_t fenv;
  unsigned long long l;
} fenv_union_t;


static inline int
__fesetround_inline (int round)
{
  if ((unsigned int) round < 2)
    {
       asm volatile ("mtfsb0 30");
       if ((unsigned int) round == 0)
         asm volatile ("mtfsb0 31");
       else
         asm volatile ("mtfsb1 31");
    }
  else
    {
       asm volatile ("mtfsb1 30");
       if ((unsigned int) round == 2)
         asm volatile ("mtfsb0 31");
       else
         asm volatile ("mtfsb1 31");
    }

  return 0;
}

/* Same as __fesetround_inline, however without runtime check to use DFP
   mtfsfi syntax (as relax_fenv_state) or if round value is valid.  */
static inline void
__fesetround_inline_nocheck (const int round)
{
  asm volatile ("mtfsfi 7,%0" : : "i" (round));
}

/* Definitions of all the FPSCR bit numbers */
enum {
  FPSCR_FX = 0,    /* exception summary */
  FPSCR_FEX,       /* enabled exception summary */
  FPSCR_VX,        /* invalid operation summary */
  FPSCR_OX,        /* overflow */
  FPSCR_UX,        /* underflow */
  FPSCR_ZX,        /* zero divide */
  FPSCR_XX,        /* inexact */
  FPSCR_VXSNAN,    /* invalid operation for sNaN */
  FPSCR_VXISI,     /* invalid operation for Inf-Inf */
  FPSCR_VXIDI,     /* invalid operation for Inf/Inf */
  FPSCR_VXZDZ,     /* invalid operation for 0/0 */
  FPSCR_VXIMZ,     /* invalid operation for Inf*0 */
  FPSCR_VXVC,      /* invalid operation for invalid compare */
  FPSCR_FR,        /* fraction rounded [fraction was incremented by round] */
  FPSCR_FI,        /* fraction inexact */
  FPSCR_FPRF_C,    /* result class descriptor */
  FPSCR_FPRF_FL,   /* result less than (usually, less than 0) */
  FPSCR_FPRF_FG,   /* result greater than */
  FPSCR_FPRF_FE,   /* result equal to */
  FPSCR_FPRF_FU,   /* result unordered */
  FPSCR_20,        /* reserved */
  FPSCR_VXSOFT,    /* invalid operation set by software */
  FPSCR_VXSQRT,    /* invalid operation for square root */
  FPSCR_VXCVI,     /* invalid operation for invalid integer convert */
  FPSCR_VE,        /* invalid operation exception enable */
  FPSCR_OE,        /* overflow exception enable */
  FPSCR_UE,        /* underflow exception enable */
  FPSCR_ZE,        /* zero divide exception enable */
  FPSCR_XE,        /* inexact exception enable */
#ifdef _ARCH_PWR6
  FPSCR_29,        /* Reserved in ISA 2.05  */
#else
  FPSCR_NI         /* non-IEEE mode (typically, no denormalised numbers) */
#endif /* _ARCH_PWR6 */
  /* the remaining two least-significant bits keep the rounding mode */
};

static inline int
fenv_reg_to_exceptions (unsigned long long l)
{
  int result = 0;
  if (l & (1 << (31 - FPSCR_XE)))
    result |= FE_INEXACT;
  if (l & (1 << (31 - FPSCR_ZE)))
    result |= FE_DIVBYZERO;
  if (l & (1 << (31 - FPSCR_UE)))
    result |= FE_UNDERFLOW;
  if (l & (1 << (31 - FPSCR_OE)))
    result |= FE_OVERFLOW;
  if (l & (1 << (31 - FPSCR_VE)))
    result |= FE_INVALID;
  return result;
}

#ifdef _ARCH_PWR6
  /* Not supported in ISA 2.05.  Provided for source compat only.  */
# define FPSCR_NI 29
#endif /* _ARCH_PWR6 */

/* This operation (i) sets the appropriate FPSCR bits for its
   parameter, (ii) converts sNaN to the corresponding qNaN, and (iii)
   otherwise passes its parameter through unchanged (in particular, -0
   and +0 stay as they were).  The `obvious' way to do this is optimised
   out by gcc.  */
#define f_wash(x) \
   ({ double d; asm volatile ("fmul %0,%1,%2" \
			      : "=f"(d) \
			      : "f" (x), "f"((float)1.0)); d; })
#define f_washf(x) \
   ({ float f; asm volatile ("fmuls %0,%1,%2" \
			     : "=f"(f) \
			     : "f" (x), "f"((float)1.0)); f; })

#endif /* fenv_libc.h */
