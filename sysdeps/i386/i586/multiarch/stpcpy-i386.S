#define __stpcpy __stpcpy_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef libc_hidden_def
#define libc_hidden_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
#include <sysdeps/i386/stpcpy.S>
