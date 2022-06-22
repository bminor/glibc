#ifndef RAWMEMCHR
# define RAWMEMCHR	__rawmemchr_evex
#endif
#define USE_AS_RAWMEMCHR	1
#define MEMCHR	RAWMEMCHR

#include "memchr-evex.S"
