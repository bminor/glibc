#define USE_AS_MEMMOVE
#define USE_AS_BCOPY
#define MEMCPY		__bcopy_erms
#include "memcpy-erms.S"
