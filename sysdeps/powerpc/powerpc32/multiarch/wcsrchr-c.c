#include <wchar.h>

#ifndef NOT_IN_libc
# define WCSRCHR  __wcsrchr_ppc32
#endif

extern __typeof (wcsrchr) __wcsrchr_ppc32;

#include "wcsmbs/wcsrchr.c"
