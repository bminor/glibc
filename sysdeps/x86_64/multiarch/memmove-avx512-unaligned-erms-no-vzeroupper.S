#ifdef HAVE_AVX512_ASM_SUPPORT
# define VEC_SIZE	64
# define VEC(i)		zmm##i
# define VMOVU		vmovdqu64
# define VMOVA		vmovdqa64
# define VZEROUPPER

# define SECTION(p)		p##.avx512
# define MEMMOVE_SYMBOL(p,s)	p##_avx512_no_vzeroupper_##s

# include "memmove-vec-unaligned-erms.S"
#endif
