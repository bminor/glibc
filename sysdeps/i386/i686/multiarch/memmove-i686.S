#include <sysdeps/i386/multiarch/memmove-i686.S>

#ifdef SHARED
	.globl __GI_memmove
	.hidden __GI_memmove
	__GI_memmove = __memmove_i686
#endif
