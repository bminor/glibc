#include <init-arch.h>
#define __stpncpy __stpncpy_i386
#undef libc_hidden_def
#define libc_hidden_def(name)
#undef weak_alias
#define weak_alias(name, aliasname)
#include <sysdeps/i386/stpncpy.S>

#ifdef SHARED
	.globl __GI___stpncpy
	.hidden __GI___stpncpy
	__GI___stpncpy = __stpncpy_i386
#endif
