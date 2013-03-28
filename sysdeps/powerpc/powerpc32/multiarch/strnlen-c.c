#define STRNLEN  __strnlen_ppc32
#ifdef SHARED
# undef libc_hidden_def
# define libc_hidden_def(name)  \
  __hidden_ver1 (__strnlen_ppc32, __GI_strnlen, __strnlen_ppc32);
#endif

#include "string/strnlen.c"
