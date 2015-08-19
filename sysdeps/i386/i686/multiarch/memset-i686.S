#include <sysdeps/i386/multiarch/memset-i686.S>

#ifdef SHARED
	.globl __GI_memset
	.hidden __GI_memset
	__GI_memset = __memset_i686
#endif
