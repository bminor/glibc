#if IS_IN (libc)
# define VEC_SIZE	64
# define XMM0		xmm16
# define XMM1		xmm17
# define YMM0		ymm16
# define YMM1		ymm17
# define VEC0		zmm16
# define VEC1		zmm17
# define VEC2		zmm18
# define VEC3		zmm19
# define VEC4		zmm20
# define VEC5		zmm21
# define VEC6		zmm22
# define VEC7		zmm23
# define VEC8		zmm24
# define VEC(i)		VEC##i
# define VMOVNT		vmovntdq
# define VMOVU		vmovdqu64
# define VMOVA		vmovdqa64
# define VZEROUPPER

# define SECTION(p)		p##.evex512
# define MEMMOVE_SYMBOL(p,s)	p##_avx512_##s

# include "memmove-vec-unaligned-erms.S"
#endif
