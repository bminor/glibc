#ifdef SHARED
# include <init-arch.h>
# if !SUPPORT_I686
#  define memmove __memmove_i386
#  define __memmove_chk __memmove_chk_i386
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name)
#  include <sysdeps/i386/memmove.S>

	.globl __GI_memmove
	.hidden __GI_memmove
	__GI_memmove = __memmove_i386
# endif
#endif
