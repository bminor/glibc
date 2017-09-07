#ifndef _UTIME_H

#include <io/utime.h>

#ifndef _ISOMAC
libc_hidden_proto (utime)
#endif

/* Structure describing file times, 64-bit time version.  */
struct __utimbuf64
  {
    __time64_t actime;		/* Access time.  */
    __time64_t modtime;		/* Modification time.  */
  };

#endif /* utime.h */
