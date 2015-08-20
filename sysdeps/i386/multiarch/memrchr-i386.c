#if IS_IN (libc)
# define MEMRCHR  __memrchr_i386
# include <string.h>
extern void *__memrchr_i386 (const void *, int, size_t);
#endif

#include "string/memrchr.c"
