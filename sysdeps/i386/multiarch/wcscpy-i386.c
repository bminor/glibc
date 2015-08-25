#if IS_IN (libc)
# define wcscpy  __wcscpy_i386
#endif

#include "wcsmbs/wcscpy.c"
