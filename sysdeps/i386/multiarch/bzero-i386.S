#include <sysdeps/i386/i586/multiarch/bzero-i386.S>

#ifdef SHARED
	.globl __GI_bzero
	.hidden __GI_bzero
	__GI_bzero = __bzero_i386
#endif
