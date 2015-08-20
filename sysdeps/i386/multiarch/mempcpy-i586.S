#ifdef SHARED
# include <init-arch.h>
# if !SUPPORT_I686
#  define __mempcpy __mempcpy_i586
#  define __mempcpy_chk __mempcpy_chk_i586
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name)
#  undef libc_hidden_def
#  define libc_hidden_def(name)
#  undef weak_alias
#  define weak_alias(name, aliasname)
#  include <sysdeps/i386/i586/mempcpy.S>

#  if SUPPORT_I586
	.globl __GI_mempcpy
	.hidden __GI_mempcpy
	__GI_mempcpy = __mempcpy_i586
	.globl __GI___mempcpy
	.hidden __GI___mempcpy 
	__GI___mempcpy = __mempcpy_i586
#  endif
# endif
#endif
