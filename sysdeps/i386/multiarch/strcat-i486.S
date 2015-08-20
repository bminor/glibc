#include <init-arch.h>
#define strcat __strcat_i486
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)

#include <sysdeps/i386/i486/strcat.S>

#ifdef SHARED
	.globl __GI_strcat
	.hidden __GI_strcat
	__GI_strcat = __strcat_i486
#endif
