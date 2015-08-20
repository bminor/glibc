#include <init-arch.h>
#if !SUPPORT_I686
# define bcopy __bcopy_i386
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# undef weak_alias
# define weak_alias(name, aliasname)
# include <sysdeps/i386/bcopy.S>

# ifdef SHARED
	.globl __GI_bcopy
	.hidden __GI_bcopy
	__GI_bcopy = __bcopy_i386
# endif
#endif
