/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

/* Signal handlers are actually called:
   void handler (int sig, int code, struct sigcontext *scp);  */

/* State of this thread when the signal was taken.  */
struct sigcontext
{
  int sc_onstack;		/* Nonzero if running on sigstack.  */
  sigset_t sc_mask;		/* Blocked signals to restore.  */

  /* Port this thread is doing an interruptible RPC on.  */
  unsigned long int sc_intr_port;
  
  /* Segment registers (not used).  */
  int sc_gs;
  int sc_fs;
  int sc_es;
  int sc_ds;
  int sc_ss;
  
  /* "General" registers.  */
  int sc_edi;
  int sc_esi;
  int sc_ebp;
  int sc_esp;			/* not used */
  int sc_ebx;
  int sc_edx;
  int sc_ecx;
  int sc_eax;
  
  int sc_trapno;		/* not used */
  int sc_err;			/* not used */
  
  int sc_eip;			/* instruction pointer */
   
  int sc_cs;			/* not used */
  
  int sc_efl;			/* flags */
  int sc_uesp;			/* THIS stack pointer is used */
};


/* Codes for SIGFPE.  */
#define FPE_INTOVF_TRAP		0x1 /* integer overflow */
#define FPE_INTDIV_FAULT	0x2 /* integer divide by zero */
#define FPE_FLTOVF_FAULT	0x3 /* floating overflow */
#define FPE_FLTDIV_FAULT	0x4 /* floating divide by zero */
#define FPE_FLTUND_FAULT	0x5 /* floating underflow */
#define FPE_SUBRNG_FAULT	0x7 /* BOUNDS instruction failed */
#define FPE_FLTDNR_FAULT	0x8 /* denormalized operand */
#define FPE_FLTINX_FAULT	0x9 /* floating loss of precision */
#define FPE_EMERR_FAULT		0xa /* mysterious emulation error 33 */
#define FPE_EMBND_FAULT		0xb /* emulation BOUNDS instruction failed */

/* Codes for SIGILL.  */
#define ILL_INVOPR_FAULT	0x1 /* invalid operation */
#define ILL_STACK_FAULT		0x2 /* fault on microkernel stack access */
#define ILL_FPEOPR_FAULT	0x3 /* invalid floating operation */

/* Codes for SIGTRAP.  */
#define DBG_SINGLE_TRAP		0x1 /* single step */
#define DBG_BRKPNT_FAULT	0x2 /* breakpoint instruction */
