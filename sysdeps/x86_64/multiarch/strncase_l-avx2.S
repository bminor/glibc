#define AS_STRCASECMP
#define AS_STRNCMP
#define USE_AVX2
#define __strncasecmp_sse2_unaligned __strncasecmp_avx2
#define STRCMP __strncasecmp_avx2_l
#include "strcmp-sse2-unaligned.S"
