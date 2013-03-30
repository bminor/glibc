#define RAWMEMCHR  __rawmemchr_ppc32
#ifdef SHARED
# undef libc_hidden_def
# define libc_hidden_def(name)  \
  __hidden_ver1 (__rawmemchr_ppc32, __GI___rawmemchr, __rawmemchr_ppc32);
#endif

#include "string/rawmemchr.c"
