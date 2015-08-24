#ifdef SHARED
# define strpbrk __strpbrk_i386
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
#endif

#include <sysdeps/i386/strpbrk.S>

#ifdef SHARED
	.globl __GI_strpbrk
	.hidden __GI_strpbrk
	__GI_strpbrk = __strpbrk_i386
#endif
