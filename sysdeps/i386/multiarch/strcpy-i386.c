#include <init-arch.h>
#if !SUPPORT_I586
# undef libc_hidden_builtin_def
# ifdef SHARED
#  define libc_hidden_builtin_def(name)  \
    __hidden_ver1 (__strcpy_i386, __GI_strcpy, __strcpy_i386);
# else
#  define libc_hidden_builtin_def(name)
# endif
#endif

#define STRCPY __strcpy_i386
#include "string/strcpy.c"
