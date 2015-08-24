#include <sysdeps/i386/i586/multiarch/strlen-i386.S>

#ifdef SHARED
	.globl __GI_strlen
	.hidden __GI_strlen
	__GI_strlen = __strlen_i386
#endif
