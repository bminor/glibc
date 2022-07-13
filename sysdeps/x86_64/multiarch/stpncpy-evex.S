#ifndef STPNCPY
# define STPNCPY	__stpncpy_evex
#endif

#define USE_AS_STPCPY
#define USE_AS_STRNCPY
#define STRCPY	STPNCPY
#include "strcpy-evex.S"
