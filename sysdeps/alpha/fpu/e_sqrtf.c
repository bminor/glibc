#include <shlib-compat.h>

#define __ieee754_sqrtf __ieee754_sqrtf
#include <sysdeps/ieee754/flt-32/e_sqrtf.c>

/* Work around forgotten symbol in alphaev6 build.  */
#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
strong_alias(__ieee754_sqrtf, __sqrtf_dummy)
compat_symbol (libm, __sqrtf_dummy, __sqrtf_finite, GLIBC_2_15);
#endif
#if SHLIB_COMPAT (libm, GLIBC_2_18, GLIBC_2_31)
compat_symbol (libm, __ieee754_sqrtf, __sqrtf_finite, GLIBC_2_18);
#endif
