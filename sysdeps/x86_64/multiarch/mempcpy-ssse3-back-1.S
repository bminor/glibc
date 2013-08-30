/* optimized version of mempcpy without any checks or copying bounds.  */
#define USE_AS_MEMPCPY
#define MEMCPY chkp_mempcpy_nobnd_nochk
#undef __CHKP__
#undef __CHKWR__
#include "memcpy-ssse3-back.S"
