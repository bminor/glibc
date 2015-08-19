#include <sysdeps/i386/i586/multiarch/memset-i386.S>

#ifdef SHARED
	.globl __GI_memset
	.hidden __GI_memset
	__GI_memset = __memset_i386
#endif
