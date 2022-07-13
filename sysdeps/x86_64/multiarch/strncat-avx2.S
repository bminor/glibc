#ifndef STRNCAT
# define STRNCAT	__strncat_avx2
#endif

#define USE_AS_STRNCAT
#define STRCAT	STRNCAT
#include "strcat-avx2.S"
