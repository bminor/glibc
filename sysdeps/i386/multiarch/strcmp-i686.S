#ifdef SHARED
# define strcmp __strcmp_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# include <sysdeps/i386/i686/strcmp.S>
#endif
