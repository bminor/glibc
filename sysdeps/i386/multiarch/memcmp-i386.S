#include <init-arch.h>
#if !SUPPORT_I686
# define memcmp __memcmp_i386
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# undef weak_alias
# define weak_alias(name, aliasname)
# include <sysdeps/i386/memcmp.S>

# ifdef SHARED
	.globl __GI_memcmp
	.hidden __GI_memcmp
	__GI_memcmp = __memcmp_i386
# endif
#endif
