#include <sysdeps/x86/isa-level.h>

#if MINIMUM_X86_ISA_LEVEL != SSE4_1_X86_ISA_LEVEL
# define __modff __modff_sse41
#endif

#include <sysdeps/ieee754/flt-32/s_modff.c>
