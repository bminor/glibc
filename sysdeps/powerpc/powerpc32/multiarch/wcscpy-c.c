#include <wchar.h>

#ifndef NOT_IN_libc
# define WCSCPY  __wcscpy_ppc32
#endif

extern __typeof (wcscpy) __wcscpy_ppc32;

#include "wcsmbs/wcscpy.c"
