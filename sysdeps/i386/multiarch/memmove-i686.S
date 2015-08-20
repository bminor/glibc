#ifdef SHARED
# include <init-arch.h>
# define memmove __memmove_i686
# define __memmove_chk __memmove_chk_i686
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# include <sysdeps/i386/i686/memmove.S>

# if SUPPORT_I686
	.globl __GI_memmove
	.hidden __GI_memmvoe
	__GI_memmove = __memmove_i686
# endif
#endif
