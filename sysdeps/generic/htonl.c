#include <ansidecl.h>
#include <netinet/in.h>

#undef	htonl

unsigned long int
DEFUN(htonl, (x), unsigned long int x)
{
  return x;
}
