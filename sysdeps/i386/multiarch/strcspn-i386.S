#define strcspn __strcspn_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/strcspn.S>

	.globl __GI_strcspn
	.hidden __GI_strcspn
	__GI_strcspn = __strcspn_i386
