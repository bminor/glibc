#define STRCMP __strncasecmp_l
#define USE_AS_STRNCASECMP_L
#include "multiarch/strcmp-sse2.S"

libc_hidden_builtin_def (STRCMP)
weak_alias (__strncasecmp_l, strncasecmp_l)
libc_hidden_def (strncasecmp_l)
