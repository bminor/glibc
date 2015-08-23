#include <sysdeps/i386/i586/multiarch/strchr-i386.S>

#ifdef SHARED
	.globl __GI_strchr
	.hidden __GI_strchr
	__GI_strchr = __strchr_i386
#endif
