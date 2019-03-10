#ifndef __ino_t_defined
#define __ino_t_defined 1

#include <bits/types.h>

/* Type for file serial numbers (POSIX).  */
# ifndef __USE_FILE_OFFSET64
typedef __ino_t ino_t;
# else
typedef __ino64_t ino_t;
# endif

#endif
