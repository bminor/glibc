#ifndef __off_t_defined
#define __off_t_defined 1

#include <bits/types.h>

/* Type for file sizes and offsets within a file (POSIX).  */
# ifndef __USE_FILE_OFFSET64
typedef __off_t off_t;
# else
typedef __off64_t off_t;
# endif

#endif
