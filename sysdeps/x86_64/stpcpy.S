#define STRCPY __stpcpy

#include "multiarch/stpcpy-sse2.S"

weak_alias (__stpcpy, stpcpy)
libc_hidden_def (__stpcpy)
libc_hidden_builtin_def (stpcpy)
