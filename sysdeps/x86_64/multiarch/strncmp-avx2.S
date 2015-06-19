#define USE_AVX2
#define AS_STRNCMP
#define STRCMP __strncmp_avx2
#include "strcmp-sse2-unaligned.S"
