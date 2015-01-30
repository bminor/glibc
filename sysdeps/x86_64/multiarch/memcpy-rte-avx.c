#if IS_IN (libc) && defined SHARED
#define RTE_MACHINE_CPUFLAG_AVX2
#define rte_memcpy  __memcpy_rte_avx
#include "rte_memcpy.h"
#endif
