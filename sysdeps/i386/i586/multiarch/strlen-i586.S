#include <sysdeps/i386/multiarch/strlen-i586.S>

#ifdef SHARED
	.globl __GI_strlen
	.hidden __GI_strlen
	__GI_strlen = __strlen_i586
#endif
