#ifndef STPNCPY
# define STPNCPY	__stpncpy_avx2
#endif

#define USE_AS_STPCPY
#define USE_AS_STRNCPY
#define STRCPY	STPNCPY
#include "strcpy-avx2.S"
