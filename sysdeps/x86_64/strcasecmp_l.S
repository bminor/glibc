/* Symbols = __strcasecmp_l and __strcasecmp.  */

#include "multiarch/strcasecmp_l-sse2.S"

libc_hidden_builtin_def (__strcasecmp_l)

weak_alias (__strcasecmp_l, strcasecmp_l)
libc_hidden_def (strcasecmp_l)

weak_alias (__strcasecmp, strcasecmp)
libc_hidden_def (__strcasecmp)
