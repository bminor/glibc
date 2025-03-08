#define __ieee754_sinh __ieee754_sinh_fma
#define __ieee754_exp __ieee754_exp_fma
#define __expm1 __expm1_fma

/* NB: __expm1 may be expanded to __expm1_fma in the following
   prototypes.  */
extern long double __expm1l (long double);
extern long double __expm1f128 (long double);

#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/e_sinh.c>
