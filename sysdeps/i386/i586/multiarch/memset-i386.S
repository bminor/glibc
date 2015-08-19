#define memset __memset_i386
#define __memset_chk __memset_chk_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/memset.S>
