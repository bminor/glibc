#define STRCMP	__strncmp_avx2
#define USE_AS_STRNCMP 1
#define OVERFLOW_STRCMP __strcmp_avx2
#include "strcmp-avx2.S"
