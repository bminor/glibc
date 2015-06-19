#define AS_STRCASECMP
#define AS_STRNCMP
#define STRCMP __strncasecmp_sse2_unaligned_l
#include "strcmp-sse2-unaligned.S"
