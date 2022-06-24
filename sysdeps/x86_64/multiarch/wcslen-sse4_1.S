#define AS_WCSLEN
#define strlen	__wcslen_sse4_1
#define SECTION(p)	p##.sse4.1

#include "strlen-vec.S"
