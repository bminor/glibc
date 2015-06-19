#define USE_AVX2
#define STRCMP __strcmp_avx2
#include "strcmp-sse2-unaligned.S"
