#ifdef SHARED
# include <init-arch.h>
# if !SUPPORT_I586 && !SUPPORT_I686
#  define memcpy __memcpy_i386
#  define __memcpy_chk __memcpy_chk_i386
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name)
#  include <sysdeps/i386/memcpy.S>

	.globl __GI_memcpy
	.hidden __GI_memcpy
	__GI_memcpy = __memcpy_i386
# endif
#endif
