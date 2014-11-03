#ifndef	_SYS_POLL_H
# include <io/sys/poll.h>

#ifndef _ISOMAC
extern int __poll (struct pollfd *__fds, unsigned long int __nfds,
		   int __timeout);
libc_hidden_proto (__poll)
libc_hidden_proto (ppoll)

#include <errno.h>
#include <sys/time.h>

static inline int
__poll_noeintr (struct pollfd *__fds, unsigned long int __nfds,
		int __timeout)
{
  int __ret = __poll (__fds, __nfds, __timeout);

  if (__ret == -1 && __glibc_unlikely (errno == EINTR))
    {
      /* Handle the case where the poll() call is interrupted by a
	 signal.  We cannot just use TEMP_FAILURE_RETRY since it might
	 lead to infinite loops.  We can't tell how long poll has
	 already waited, and we can't assume the existence of a
	 higher-precision clock, but that's ok-ish: the timeout is a
	 lower bound, we just have to make sure we don't wait
	 indefinitely.  */
      struct timeval __now;
      (void) __gettimeofday (&__now, NULL);

      long int __end = __now.tv_sec * 1000 + __timeout
	+ (__now.tv_usec + 500) / 1000;

      while (1)
	{
	  __ret = __poll (__fds, __nfds, __timeout);
	  if (__ret != -1 || errno != EINTR)
	    break;

	  /* Recompute the timeout time.  */
	  (void) __gettimeofday (&__now, NULL);
	  __timeout = __end - __now.tv_sec * 1000
	    - (__now.tv_usec + 500) / 1000;
	}
    }

  return __ret;
}
#endif /* _ISOMAC */

#endif
