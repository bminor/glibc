#include <math_private.h>

#define __modf __modf_avx
#define trunc __trunc

#include <sysdeps/ieee754/dbl-64/s_modf.c>
