#define STRNLEN  __strnlen_i386
#ifdef SHARED
# undef libc_hidden_def
# define libc_hidden_def(name)  \
    __hidden_ver1 (__strnlen_i386, __GI_strnlen, __strnlen_i386); \
    strong_alias (__strnlen_i386, __strnlen_i386_1); \
    __hidden_ver1 (__strnlen_i386_1, __GI___strnlen, __strnlen_i386_1);
#endif

#include "string/strnlen.c"
