#ifndef STRNCPY
# define STRNCPY	__strncpy_evex
#endif

#define USE_AS_STRNCPY
#define STRCPY	STRNCPY
#include "strcpy-evex.S"
