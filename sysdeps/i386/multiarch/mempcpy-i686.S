#ifdef SHARED
# define __mempcpy __mempcpy_i686
# define __mempcpy_chk __mempcpy_chk_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# undef libc_hidden_def
# define libc_hidden_def(name)
# undef weak_alias
# define weak_alias(name, aliasname)
# include <sysdeps/i386/i686/mempcpy.S>
#endif
