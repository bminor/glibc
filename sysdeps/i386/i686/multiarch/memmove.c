#include <stddef.h>

void *
__memmove (void *dst, const void *src, size_t n)
{
  const char *s = src;
  char *d = dst;
  void *ret = dst;
  size_t offset_src = ((size_t) s) & (sizeof(size_t) - 1);
  size_t offset_dst = ((size_t) d) & (sizeof(size_t) - 1);

  if (offset_src != offset_dst)
  {
    if (s < d)
    {
      // backward copying
      d += n;
      s += n;
      while (n--)
        *--d = *--s;
    }
    else
      // forward copying
      while (n--)
        *d++ = *s++;
  }
  else
  {
    if (s < d)
    {
		offset_src = (offset_src + (size_t)src) & (sizeof(size_t) - 1);
      // backward copying
      d += n;
      s += n;
      while (n-- && offset_src--)
        *--d = *--s;
      n++;
      if (!n) return ret;
      void **d1 = (void **)d;
      void **s1 = (void **)s;
      while (n >= sizeof(void *))
      {
        n -= sizeof(void *);
        *--d1 = *--s1;
      }
      s = (char *)s1;
      d = (char *)d1;
      while (n--)
        *--d = *--s;
    }
    else
    {
		if (offset_src) offset_src = sizeof(size_t) - offset_src;
      // forward copying
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
  }
  return ret;
}

weak_alias (__memmove, __GI_memmove)
weak_alias (__memmove, memmove)
