#include <sysdeps/i386/multiarch/bcopy-i686.S>

#ifdef SHARED
	.globl __GI_bcopy
	.hidden __GI_bcopy
	__GI_bcopy = __bcopy_i686
#endif
