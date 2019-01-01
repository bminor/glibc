/* ptrace register data format definitions.  C-SKY version.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_USER_H
#define _SYS_USER_H  1

struct user_fpregs {
  unsigned long fsr;		/* FPU status reg.  */
  unsigned long fesr;		/* FPU exception status reg.  */
  unsigned long fp[32];		/* FPU general regs.  */
};

struct user_regs {
  unsigned long int uregs[34];	/* CSKY V2 has 32 general rgister.  */
};

/* When the kernel dumps core, it starts by dumping the user struct -
   this will be used by gdb to figure out where the data and stack segments
   are within the file, and what virtual addresses to use.  */
struct user{
/* We start with the registers, to mimic the way that "memory" is returned
   from the ptrace(3,...) function.  */
  struct user_regs    regs;        /* The registers are actually stored.  */
  int                 u_fpvalid;   /* True if math co-processor being used.  */

/* The rest of this junk is to help gdb figure out what goes where.  */
  unsigned long int   u_tsize;     /* Text segment size (pages).  */
  unsigned long int   u_dsize;     /* Data segment size (pages).  */
  unsigned long int   u_ssize;     /* Stack segment size (pages).  */
  unsigned long       start_code;  /* Starting virtual address of text.  */
  unsigned long       start_stack; /* Starting virtual address of stack area.
				      This is actually the bottom of the stack
				      the top of the stack is always found in
				      the esp register.  */
  long int            signal;      /* Signal that caused the core dump.  */
  int                 reserved;    /* No longer used.  */
  struct user_regs *  u_ar0;       /* Used by gdb to help find the values
				      for the registers.  */
  unsigned long       magic;       /* To uniquely identify a core file.  */
  char                u_comm[32];  /* User command that was responsible.  */
  struct user_fpregs  u_fp;
  struct user_fpregs* u_fpstate;   /* Math Co-processor pointer.  */
};

#endif /* _SYS_USER_H */
