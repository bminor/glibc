#include <isa-level.h>

#if ISA_SHOULD_BUILD (3)

# define VEC_SIZE	32
# define VEC(i)		ymm##i
# define VMOVNT		vmovntdq
# define VMOVU		vmovdqu
# define VMOVA		vmovdqa
# define MOV_SIZE	4

# define SECTION(p)		p##.avx

# ifndef MEMMOVE_SYMBOL
#  define MEMMOVE_SYMBOL(p,s)	p##_avx_##s
# endif

# include "memmove-vec-unaligned-erms.S"

# if MINIMUM_X86_ISA_LEVEL == 3
#  include "memmove-shlib-compat.h"
# endif
#endif
