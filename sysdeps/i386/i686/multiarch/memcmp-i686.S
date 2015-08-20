#include <sysdeps/i386/multiarch/memcmp-i686.S>

#ifdef SHARED
	.globl __GI_memcmp
	.hidden __GI_memcmp
	__GI_memcmp = __memcmp_i686
#endif
