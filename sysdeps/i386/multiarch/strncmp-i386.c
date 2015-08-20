#ifdef SHARED
# define STRNCMP __strncmp_i386
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)  \
    __hidden_ver1 (__strncmp_i386, __GI_strncmp, __strncmp_i386);
#endif

#include "string/strncmp.c"
