#define USE_AS_MEMPCPY

#if defined __CHKP__ || defined __CHKWR__
/* version of mempcpy with no copying of bounds support
   if there are pointers in the source buffer. */
# define MEMCPY  chkp_mempcpy_nobnd
#else
# define MEMCPY	__mempcpy_ssse3_back
#endif

#define MEMCPY_CHK __mempcpy_chk_ssse3_back
#include "memcpy-ssse3-back.S"
