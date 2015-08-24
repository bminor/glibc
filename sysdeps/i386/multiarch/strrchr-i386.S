#define strrchr __strrchr_i386
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef libc_hidden_def
#define libc_hidden_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
#include <sysdeps/i386/strrchr.S>

#ifdef SHARED
	.globl __GI_strrchr
	.hidden __GI_strrchr
	__GI_strrchr = __strrchr_i386
#endif
