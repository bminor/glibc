#include <sysdeps/i386/multiarch/memcpy-i686.S>

#ifdef SHARED
	.globl __GI_memcpy
	.hidden __GI_memcpy
	__GI_memcpy = __memcpy_i686
#endif
