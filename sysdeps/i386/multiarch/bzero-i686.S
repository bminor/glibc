#include <init-arch.h>
#define __bzero __bzero_i686
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
#include <sysdeps/i386/i686/bzero.S>

#if defined SHARED && SUPPORT_I686
	.globl __GI_bzero
	.hidden __GI_bzero
	__GI_bzero = __bzero_i686
#endif
