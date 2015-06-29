#define STRPBRK __strpbrk_sse2
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(x)
#include "string/strpbrk.c"
