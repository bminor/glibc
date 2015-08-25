#include <wchar.h>

#if IS_IN (libc)
# define WMEMCMP  __wmemcmp_i386
#endif

extern __typeof (wmemcmp) __wmemcmp_i386;

#include "wcsmbs/wmemcmp.c"
