#define USE_AS_STPCPY
#define STRCPY __stpncpy_sse2_unaligned
#include "strncpy-sse2-unaligned.S"
