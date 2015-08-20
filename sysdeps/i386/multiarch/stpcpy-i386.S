#include <sysdeps/i386/i586/multiarch/stpcpy-i386.S>

#ifdef SHARED
	.globl __GI_stpcpy
	.hidden __GI_stpcpy
	__GI_stpcpy = __stpcpy_i386
#endif
