/* Don't define PTHREAD_STACK_MIN to sysconf (_SC_THREAD_STACK_MIN) for
   glibc build.  */
#if !defined _ISOMAC
# undef __USE_DYNAMIC_STACK_SIZE
#endif

#include_next <bits/local_lim.h>
