#define USE_AS_MEMPCPY
#define __memcpy_sse2		__mempcpy_sse2
#define __memcpy_chk_sse2	__mempcpy_chk_sse2
#include "memcpy-sse2.S"
