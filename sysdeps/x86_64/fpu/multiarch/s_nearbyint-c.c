#include <math.h>

extern typeof (__nearbyint) __nearbyint_c;
libm_hidden_proto (__nearbyint_c)

#undef __nearbyint
#define __nearbyint __nearbyint_c
#include <sysdeps/ieee754/dbl-64/wordsize-64/s_nearbyint.c>
