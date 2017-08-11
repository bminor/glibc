/* Architecture-specific adjustments to siginfo_t.  ia64 version.  */
#ifndef _BITS_SIGINFO_ARCH_H

#define __SI_HAVE_SIGSYS 0

#ifdef __USE_GNU
# define __SI_SIGFAULT_ADDL			\
  int si_imm;					\
  unsigned int si_segvflags;			\
  unsigned long int si_isr;
#else
# define __SI_SIGFAULT_ADDL			\
  int __si_imm;					\
  unsigned int __si_segvflags;			\
  unsigned long int __si_isr;
#endif

#endif
