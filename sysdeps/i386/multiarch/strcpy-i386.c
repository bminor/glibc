#include <init-arch.h>
#undef libc_hidden_builtin_def
#if defined SHARED && MINIMUM_ISA != 586
# define libc_hidden_builtin_def(name)  \
   __hidden_ver1 (__strcpy_i386, __GI_strcpy, __strcpy_i386);
#else
# define libc_hidden_builtin_def(name)
#endif

#define STRCPY __strcpy_i386
#include "string/strcpy.c"
