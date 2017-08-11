#ifndef __siginfo_t_defined
#define __siginfo_t_defined 1

#include <bits/wordsize.h>
#include <bits/types.h>
#include <bits/types/__sigval_t.h>

#define __SI_MAX_SIZE	128
#if __WORDSIZE == 64
# define __SI_PAD_SIZE	((__SI_MAX_SIZE / sizeof (int)) - 4)
#else
# define __SI_PAD_SIZE	((__SI_MAX_SIZE / sizeof (int)) - 3)
#endif

/* Some fields of siginfo_t have architecture-specific variations.  */
#include <bits/siginfo-arch.h>
#ifndef __SI_ALIGNMENT
# define __SI_ALIGNMENT		/* nothing */
#endif
#ifndef __SI_BAND_TYPE
# define __SI_BAND_TYPE		long int
#endif
#ifndef __SI_CLOCK_T
# define __SI_CLOCK_T		__clock_t
#endif
#ifndef __SI_ERRNO_THEN_CODE
# define __SI_ERRNO_THEN_CODE	1
#endif
#ifndef __SI_HAVE_SIGSYS
# define __SI_HAVE_SIGSYS	1
#endif
#ifndef __SI_SIGFAULT_ADDL
# define __SI_SIGFAULT_ADDL	/* nothing */
#endif

typedef struct
  {
    int si_signo;		/* Signal number.  */
#if __SI_ERRNO_THEN_CODE
    int si_errno;		/* If non-zero, an errno value associated with
				   this signal, as defined in <errno.h>.  */
    int si_code;		/* Signal code.  */
#else
    int si_code;
    int si_errno;
#endif
#if __WORDSIZE == 64
    int __pad0;			/* Explicit padding.  */
#endif

    union
      {
	int _pad[__SI_PAD_SIZE];

	 /* Signals sent by kill or sigqueue.  si_sigval is only valid for
	    sigqueue.  */
	struct
	  {
	    __pid_t si_pid;	/* Sending process ID.  */
	    __uid_t si_uid;	/* Real user ID of sending process.  */
	    __sigval_t si_value;	/* Signal value.  */
	  };

	/* POSIX.1b timers.  'si_sigval' (above) is also valid.  */
	struct
	  {
	    int si_timerid;	/* Timer ID.  */
	    int si_overrun;	/* Overrun count.  */
	    __sigval_t si_sigval;	/* Signal value.  */
	  };

	/* SIGCHLD.  The first two fields overlay the si_pid and si_uid
	   fields above.  */
	struct
	  {
	    __pid_t __sigchld_si_pid;	/* Which child.	 */
	    __uid_t __sigchld_si_uid;	/* Real user ID of sending process.  */
	    int si_status;	/* Exit value or signal.  */
	    __SI_CLOCK_T si_utime;
	    __SI_CLOCK_T si_stime;
	  };

	/* SIGILL, SIGFPE, SIGSEGV, SIGBUS.  */
	struct
	  {
	    void *si_addr;	    /* Faulting insn/memory ref.  */
	    __SI_SIGFAULT_ADDL
	    short int si_addr_lsb;  /* Valid LSB of the reported address.  */
	    union
	      {
		/* used when si_code=SEGV_BNDERR */
		struct
		  {
		    void *si_lower;
		    void *si_upper;
		  };
		/* used when si_code=SEGV_PKUERR */
		__uint32_t si_pkey;
	      };
	  };

	/* SIGPOLL.  */
	struct
	  {
	    long int si_band;	/* Band event for SIGPOLL.  */
	    int si_fd;
	  };

	/* SIGSYS.  */
#if __SI_HAVE_SIGSYS
	struct
	  {
	    void *si_call_addr;	  /* Calling user insn.  */
	    int si_syscall;	  /* Triggering system call number.  */
	    unsigned int si_arch; /* AUDIT_ARCH_* of syscall.  */
	  };
#endif
      };
  } siginfo_t __SI_ALIGNMENT;


/* These field aliases are not in POSIX, and are preserved for
   backward compatibility only.  They may be removed in a future
   release.  */
#ifdef __USE_MISC
#define si_int		si_value.sival_int \
  __glibc_macro_warning("si_int is deprecated, use si_value.sival_int instead")
#define si_ptr		si_value.sival_ptr \
  __glibc_macro_warning("si_ptr is deprecated, use si_value.sival_ptr instead")
#endif

#endif
