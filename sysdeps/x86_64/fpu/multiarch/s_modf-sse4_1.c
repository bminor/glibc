#include <sysdeps/x86/isa-level.h>

#if MINIMUM_X86_ISA_LEVEL != SSE4_1_X86_ISA_LEVEL
# define __modf __modf_sse41
#endif

#include <sysdeps/ieee754/dbl-64/s_modf.c>
