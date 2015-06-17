/* Multiple versions of stpncpy
   All versions must be listed in ifunc-impl-list.c.  */
#define STRNCPY __stpncpy
#define USE_AS_STPCPY
#include "strncpy.S"

weak_alias (__stpncpy, stpncpy)
