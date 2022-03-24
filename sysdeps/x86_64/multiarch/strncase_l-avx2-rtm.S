#ifndef STRCMP
# define STRCMP	__strncasecmp_l_avx2_rtm
#endif

#define _GLABEL(x)	x ## _rtm
#define GLABEL(x)	_GLABEL(x)

#define ZERO_UPPER_VEC_REGISTERS_RETURN	\
	ZERO_UPPER_VEC_REGISTERS_RETURN_XTEST

#define VZEROUPPER_RETURN	jmp L(return_vzeroupper)

#define SECTION(p)	p##.avx.rtm
#define OVERFLOW_STRCMP	__strcasecmp_l_avx2_rtm

#include "strncase_l-avx2.S"
