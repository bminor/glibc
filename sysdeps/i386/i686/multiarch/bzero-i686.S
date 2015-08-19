#include <sysdeps/i386/multiarch/bzero-i686.S>

#ifdef SHARED
	.globl __GI_bzero
	.hidden __GI_bzero
	__GI_bzero = __bzero_i686
#endif
