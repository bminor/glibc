#include <wchar.h>

#if IS_IN (libc)
# define WCSLEN  __wcslen_i386
#endif

extern __typeof (wcslen) __wcslen_i386;

#include "wcsmbs/wcslen.c"
