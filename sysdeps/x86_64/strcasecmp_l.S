#define STRCMP __strcasecmp_l
#define USE_AS_STRCASECMP_L
#include "multiarch/strcmp-sse2.S"

libc_hidden_builtin_def (STRCMP)
weak_alias (__strcasecmp_l, strcasecmp_l)
libc_hidden_def (strcasecmp_l)
