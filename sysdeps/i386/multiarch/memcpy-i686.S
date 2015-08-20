#ifdef SHARED
# define memcpy __memcpy_i686
# define __memcpy_chk __memcpy_chk_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# include <sysdeps/i386/i686/memcpy.S>
#endif
