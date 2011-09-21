#define USE_AS_MEMMOVE
#define MEMCPY		__memmove_erms
#define MEMCPY_CHK	__memmove_chk_erms
#include "memcpy-erms.S"
