#include <init-arch.h>
#define strcpy __strcpy_i586
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/i586/strcpy.S>

#if defined SHARED && SUPPORT_I586
	.globl __GI_strcpy
	.hidden __GI_strcpy
	__GI_strcpy = __strcpy_i586
#endif
