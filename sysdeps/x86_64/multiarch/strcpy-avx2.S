#define USE_AVX2
#define AS_STRCPY
#define STPCPY __strcpy_avx2
#include "stpcpy-sse2-unaligned.S"
