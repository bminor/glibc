/* Architecture-specific adjustments to siginfo_t.  SPARC version.  */
#ifndef _BITS_SIGINFO_ARCH_H
#define _BITS_SIGINFO_ARCH_H 1

#define __SI_BAND_TYPE int

#ifdef __USE_GNU
# define __SI_SIGFAULT_ADDL  int si_trapno;
#else
# define __SI_SIGFAULT_ADDL  int __si_trapno;
#endif

#endif
