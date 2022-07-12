/* Symbols = __strncasecmp_l and __strncasecmp.  */

#include "multiarch/strncase_l-sse2.S"

libc_hidden_builtin_def (__strncasecmp_l)

weak_alias (__strncasecmp_l, strncasecmp_l)
libc_hidden_def (strncasecmp_l)

weak_alias (__strncasecmp, strncasecmp)
libc_hidden_def (__strncasecmp)
