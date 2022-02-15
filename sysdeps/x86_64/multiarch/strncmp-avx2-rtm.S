#define STRCMP	__strncmp_avx2_rtm
#define USE_AS_STRNCMP 1
#define OVERFLOW_STRCMP	__strcmp_avx2_rtm
#include "strcmp-avx2-rtm.S"
