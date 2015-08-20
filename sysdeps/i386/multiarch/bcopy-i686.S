#include <init-arch.h>
#define bcopy __bcopy_i686
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
# include <sysdeps/i386/i686/bcopy.S>

#if defined SHARED && SUPPORT_I686
	.globl __GI_bcopy
	.hidden __GI_bcopy
	__GI_bcopy = __bcopy_i686
#endif
