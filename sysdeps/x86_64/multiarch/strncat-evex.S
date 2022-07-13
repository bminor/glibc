#ifndef STRNCAT
# define STRNCAT	__strncat_evex
#endif

#define USE_AS_STRNCAT
#define STRCAT	STRNCAT
#include "strcat-evex.S"
