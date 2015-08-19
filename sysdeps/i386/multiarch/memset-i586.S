#include <init-arch.h>
#if !SUPPORT_I686
# define memset __memset_i586
# define __memset_chk __memset_chk_i586
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# include <sysdeps/i386/i586/memset.S>

# if defined SHARED && SUPPORT_I586
	.globl __GI_memset
	.hidden __GI_memset
	__GI_memset = __memset_i586
# endif
#endif
