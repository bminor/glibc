#define AS_WCSLEN
#define STRLEN	__wcslen_sse4_1
#define SECTION(p)	p##.sse4.1

#include "strlen-sse2.S"
