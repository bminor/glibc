#ifndef ____ia64_fpreg_defined
#define ____ia64_fpreg_defined

#include <features.h>

#ifdef __USE_MISC
# define __ctx(fld) fld
#else
# define __ctx(fld) __ ## fld
#endif

struct __ia64_fpreg
  {
    union
      {
	unsigned long __ctx(bits)[2];
      } __ctx(u);
  } __attribute__ ((__aligned__ (16)));

#undef __ctx

#endif /* __ia64_fpreg */
