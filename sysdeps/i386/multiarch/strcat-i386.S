#define strcat __strcat_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/strcat.S>

#ifdef SHARED
	.globl __GI_strcat
	.hidden __GI_strcat
	__GI_strcat = __strcat_i386
#endif
