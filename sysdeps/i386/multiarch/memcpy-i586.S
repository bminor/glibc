#ifdef SHARED
# include <init-arch.h>
# if !SUPPORT_I686
#  define memcpy __memcpy_i586
#  define __memcpy_chk __memcpy_chk_i586
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name)
#  include <sysdeps/i386/i586/memcpy.S>

#  if SUPPORT_I586
	.globl __GI_memcpy
	.hidden __GI_memcpy
	__GI_memcpy = __memcpy_i586
#  endif
# endif
#endif
