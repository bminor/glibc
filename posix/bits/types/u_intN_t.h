#ifndef __u_intN_t_defined
#define __u_intN_t_defined 1

#include <bits/types.h>

/* These size-specific names are used by some of the inet code.
   They were defined by ISO C without the first `_'.  */
typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;

/* Some code from BIND tests this macro to see if the types above are
   defined.  */
#define __BIT_TYPES_DEFINED__	1

#endif
