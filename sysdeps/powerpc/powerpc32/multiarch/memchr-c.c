#define MEMCHR  __memchr_ppc32
#ifdef SHARED
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)  \
  __hidden_ver1 (__memchr_ppc32, __GI_memchr, __memchr_ppc32);
#endif

#include "string/memchr.c"
