#define STRCMP strncmp
#define USE_AS_STRNCMP
#include "multiarch/strcmp-sse2.S"

libc_hidden_builtin_def (STRCMP)
