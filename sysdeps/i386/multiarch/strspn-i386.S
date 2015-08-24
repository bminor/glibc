#define strspn __strspn_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/strspn.S>

	.globl __GI_strspn
	.hidden __GI_strspn
	__GI_strspn = __strspn_i386
