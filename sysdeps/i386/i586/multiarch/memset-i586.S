#include <sysdeps/i386/multiarch/memset-i586.S>

#ifdef SHARED
	.globl __GI_memset
	.hidden __GI_memset
	__GI_memset = __memset_i586
#endif
