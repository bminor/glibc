#define AS_STRCASECMP
#define USE_AVX2
#define __strcasecmp_sse2_unaligned __strcasecmp_avx2
#define STRCMP __strcasecmp_avx2_l
#include "strcmp-sse2-unaligned.S"
