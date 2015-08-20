#include <sysdeps/i386/multiarch/stpcpy-i586.S>

#ifdef SHARED
	.globl __GI_stpcpy
	.hidden __GI_stpcpy
	__GI_stpcpy = __stpcpy_i586
#endif
