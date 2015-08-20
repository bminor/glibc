#include <init-arch.h>
#define memcmp __memcmp_i686
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
# include <sysdeps/i386/i686/memcmp.S>

#if defined SHARED && SUPPORT_I686
	.globl __GI_memcmp
	.hidden __GI_memcmp
	__GI_memcmp = __memcmp_i686
#endif
