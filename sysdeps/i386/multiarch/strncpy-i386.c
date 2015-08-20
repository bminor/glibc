#include <init-arch.h>
#undef libc_hidden_builtin_def
#ifdef SHARED
# define libc_hidden_builtin_def(name)  \
    __hidden_ver1 (__strncpy_i386, __GI_strncpy, __strncpy_i386);
#else
# define libc_hidden_builtin_def(name)
#endif

#define _HAVE_STRING_ARCH_strncpy
#define STRNCPY __strncpy_i386
#include "string/strncpy.c"
