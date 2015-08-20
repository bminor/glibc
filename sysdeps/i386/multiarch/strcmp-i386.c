#include <init-arch.h>
#if !SUPPORT_I686
# ifdef SHARED
#  define STRCMP __strcmp_i386
#  undef libc_hidden_builtin_def
#   define libc_hidden_builtin_def(name)  \
    __hidden_ver1 (__strcmp_i386, __GI_strcmp, __strcmp_i386);
# endif
# include "string/strcmp.c"
#endif
