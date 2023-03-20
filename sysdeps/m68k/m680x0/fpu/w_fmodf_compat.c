/* m68k provides an optimized __ieee752_fmodf.  */
#include <math-svid-compat.h>
#ifdef SHARED
# undef SHLIB_COMPAT
# define SHLIB_COMPAT(a, b, c) 1
# undef LIBM_SVID_COMPAT
# define LIBM_SVID_COMPAT 1
# undef compat_symbol
# define compat_symbol(a, b, c, d)
#endif
#include <math/w_fmodf_compat.c>
#ifdef SHARED
libm_alias_float (__fmod_compat, fmod)
#endif
