/* Silly pointless function MiG needs.  */

#include <ansidecl.h>
#include <string.h>

int
__mig_strncpy (char *dest, char *src, size_t len)
{
  size_t i;

  if (len == 0)
    return 0;

  for (i = 1; i < len; ++i)
    if ((*dest++ = *src++) == '\0')
      return i;

  *dest = '\0';
  return i;
}
