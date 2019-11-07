#if _MIPS_SIM == _ABI64
# define __PTHREAD_MUTEX_KIND_OFFSET     16
#else
# define __PTHREAD_MUTEX_KIND_OFFSET     12
#endif
