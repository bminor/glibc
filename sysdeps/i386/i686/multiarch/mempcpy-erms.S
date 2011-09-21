#define USE_AS_MEMPCPY
#define MEMCPY		__mempcpy_erms
#define MEMCPY_CHK	__mempcpy_chk_erms
#include "memcpy-erms.S"
