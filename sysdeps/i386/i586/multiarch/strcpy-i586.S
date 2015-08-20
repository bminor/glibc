#include <sysdeps/i386/multiarch/strcpy-i586.S>

#ifdef SHARED
	.globl __GI_strcpy
	.hidden __GI_strcpy
	__GI_strcpy = __strcpy_i586
#endif
