#define STRCMP __wcsncmp_avx2
#define USE_AS_STRNCMP 1
#define USE_AS_WCSCMP 1
#define OVERFLOW_STRCMP	__wcscmp_avx2
#include "strcmp-avx2.S"
