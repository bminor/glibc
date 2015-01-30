#if IS_IN (libc) && defined SHARED
#define rte_memcpy  __memcpy_rte_ssse3
#include "rte_memcpy.h"
#endif
