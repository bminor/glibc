#ifdef SHARED
# include <init-arch.h>
# define strcmp __strcmp_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
#endif

#include <sysdeps/i386/i686/strcmp.S>

#ifdef SHARED
	.globl __GI_strcmp
	.hidden __GI_strcmp
	__GI_strcmp = __strcmp_i686
#endif
