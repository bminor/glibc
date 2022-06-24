#define AS_WCSLEN
#define AS_STRNLEN
#define strlen	__wcsnlen_sse4_1
#define SECTION(p)	p##.sse4.1

#include "strlen-vec.S"
