#include <init-arch.h>
#ifdef SHARED
# define strcmp __strcmp_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
#endif

#if SUPPORT_I686 || defined SHARED
#include <sysdeps/i386/i686/strcmp.S>
#endif

#if defined SHARED && SUPPORT_I686
	.globl __GI_strcmp
	.hidden __GI_strcmp
	__GI_strcmp = __strcmp_i686
#endif
