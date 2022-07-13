#ifndef STRNLEN
# define STRNLEN __strnlen_evex
#endif

#define USE_AS_STRNLEN 1
#define STRLEN	STRNLEN

#include "strlen-evex.S"
