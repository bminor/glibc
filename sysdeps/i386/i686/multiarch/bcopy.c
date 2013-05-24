#include <stddef.h>

void
bcopy (const void *src, void *dst, size_t n)
{
  memmove (dst, src, n);
}
