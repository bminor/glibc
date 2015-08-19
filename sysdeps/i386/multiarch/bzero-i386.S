#include <init-arch.h>
#if !SUPPORT_I586 && !SUPPORT_I686
# define __bzero __bzero_i386
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# undef weak_alias
# define weak_alias(name, aliasname)
# include <sysdeps/i386/bzero.S>

# ifdef SHARED
	.globl __GI_bzero
	.hidden __GI_bzero
	__GI_bzero = __bzero_i386
# endif
#endif
