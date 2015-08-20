#include <sysdeps/i386/multiarch/mempcpy-i686.S>

#ifdef SHARED
	.globl __GI_mempcpy
	.hidden __GI_mempcpy
	__GI_mempcpy = __mempcpy_i686
	.globl __GI___mempcpy
	.hidden __GI___mempcpy
	__GI___mempcpy = __mempcpy_i686
#endif
