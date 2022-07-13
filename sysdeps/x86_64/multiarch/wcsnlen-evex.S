#ifndef WCSNLEN
# define WCSNLEN	__wcsnlen_evex
#endif

#define STRLEN	WCSNLEN
#define USE_AS_WCSLEN 1
#define USE_AS_STRNLEN 1

#include "strlen-evex.S"
