#if IS_IN (libc)
# define wcsrchr  __wcsrchr_i386
#endif

#include "wcsmbs/wcsrchr.c"
