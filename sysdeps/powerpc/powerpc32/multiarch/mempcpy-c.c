#define MEMPCPY  __mempcpy_ppc32
#ifdef SHARED
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)  \
  __hidden_ver1 (__mempcpy_ppc32, __GI_mempcpy, __mempcpy_ppc32);
#endif

#include "string/mempcpy.c"
