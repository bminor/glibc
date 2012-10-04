#define __mpatan __mpatan_fma4
#define __add __add_avx
#define __dvd __dvd_avx
#define __mpsqrt __mpsqrt_fma4
#define __mul __mul_fma4
#define __sub __sub_avx
#define AVOID_MPATAN_H 1
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mpatan.c>
