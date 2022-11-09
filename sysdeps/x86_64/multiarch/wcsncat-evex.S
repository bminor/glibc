#ifndef WCSCAT
# define WCSCAT	__wcsncat_evex
#endif

#define USE_AS_WCSCPY
#define USE_AS_STRCAT

#define STRNCAT	WCSCAT
#include "strncat-evex.S"
