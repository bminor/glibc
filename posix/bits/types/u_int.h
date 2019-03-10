#ifndef __u_int_defined
#define __u_int_defined 1

#include <bits/types.h>

/* Obsolete shorthand names for unsigned types, used by old BSD
   networking code and Sun RPC.  */
typedef unsigned char u_char;
typedef unsigned short int u_short;
typedef unsigned int u_int;
typedef unsigned long int u_long;
typedef __int64_t quad_t;
typedef __uint64_t u_quad_t;

#endif
