#define memset __memset_i686
#define __memset_chk __memset_chk_i686
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/i686/memset.S>
