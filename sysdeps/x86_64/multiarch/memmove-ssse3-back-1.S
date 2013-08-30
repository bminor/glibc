/* optimized version of memmove without any checks or copying bounds.  */
#define USE_AS_MEMMOVE
#define MEMCPY chkp_memmove_nobnd_nochk
#undef __CHKP__
#undef __CHKWR__
#include "memcpy-ssse3-back.S"
