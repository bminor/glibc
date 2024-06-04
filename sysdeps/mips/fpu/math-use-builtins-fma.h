#if __mips_isa_rev >= 6
# ifdef __mips_single_float
#  define USE_FMA_BUILTIN 0
# else
#  define USE_FMA_BUILTIN 1
# endif
# define USE_FMAF_BUILTIN 1
#else
# define USE_FMA_BUILTIN 0
# define USE_FMAF_BUILTIN 0
#endif
#define USE_FMAL_BUILTIN 0
#define USE_FMAF128_BUILTIN 0
