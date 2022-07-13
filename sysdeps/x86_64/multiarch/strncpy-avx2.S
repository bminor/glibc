#ifndef STRNCPY
# define STRNCPY	__strncpy_avx2
#endif

#define USE_AS_STRNCPY
#define STRCPY	STRNCPY
#include "strcpy-avx2.S"
