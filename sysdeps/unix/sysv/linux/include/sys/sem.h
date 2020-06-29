#ifndef _SYS_SEM_H
# include <sysvipc/sys/sem.h>

# ifndef _ISOMAC

__typeof__ (semtimedop) __semtimedop attribute_hidden;

#if __TIMESIZE == 64
# define __semctl64 __semctl
#else
extern int __semctl64 (int semid, int semnum, int cmd, ...);
libc_hidden_proto (__semctl64);
#endif

# endif
#endif
