#define USE_AS_MEMMOVE
#if defined __CHKP__ || defined __CHKWR__
/* version of memmove with no copying of bounds support
   if there are pointers in the source buffer. */
# define MEMCPY	   chkp_memmove_nobnd
# else
# define MEMCPY		__memmove_ssse3_back
#endif
#define MEMCPY_CHK	__memmove_chk_ssse3_back
#include "memcpy-ssse3-back.S"
