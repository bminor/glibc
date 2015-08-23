#include <sysdeps/i386/multiarch/strchr-i586.S>

#ifdef SHARED
	.globl __GI_strchr
	.hidden __GI_strchr
	__GI_strchr = __strchr_i586
#endif
