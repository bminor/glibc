#include <sysdep.h>

	.text
ENTRY(bcopy)
	xchg	%rdi, %rsi
#ifdef __CHKP__
	bndmov %bnd0, %bnd2
	bndmov %bnd1, %bnd0
	bndmov %bnd2, %bnd1
#endif
	jmp	__libc_memmove	/* Branch to IFUNC memmove.  */
END(bcopy)
