#define STRLEN __strnlen
#include "multiarch/strnlen-sse2.S"

libc_hidden_def (__strnlen)
weak_alias (__strnlen, strnlen);
libc_hidden_builtin_def (strnlen)
