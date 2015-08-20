#ifdef SHARED
# define memcpy __memcpy_i586
# define __memcpy_chk __memcpy_chk_i586
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# include <sysdeps/i386/i586/memcpy.S>
#endif
