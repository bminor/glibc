#include <stddef.h>

void *
mempcpy (void *dst, const void *src, size_t n)
{
  const char *s = src;
  char *d = dst;
  void *ret = dst + n;
  size_t offset_src = ((size_t) s) & (sizeof(size_t) - 1);
  size_t offset_dst = ((size_t) d) & (sizeof(size_t) - 1);

  if (offset_src != offset_dst)
  {
    while (n--)
      *d++ = *s++;
  }
  else
  {
	 if (offset_src) offset_src = sizeof(size_t) - offset_src;
    while (n-- && offset_src--)
      *d++ = *s++;
    n++;
    if (!n) return ret;
    void **d1 = (void **)d;
    void **s1 = (void **)s;
    while (n >= sizeof(void *))
    {
      n -= sizeof(void *);
      *d1++ = *s1++;
    }
    s = (char *)s1;
    d = (char *)d1;
    while (n--)
      *d++ = *s++;
  }
  return ret;
}

weak_alias (mempcpy, __GI_mempcpy)
weak_alias (mempcpy, __mempcpy)
