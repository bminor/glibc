/* optimized version of memcpy without any checks or copying bounds.  */
#define MEMCPY chkp_memcpy_nobnd_nochk
#undef __CHKP__
#undef __CHKWR__
#include "memcpy-ssse3-back.S"
