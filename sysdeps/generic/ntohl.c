#include <ansidecl.h>
#include <netinet/in.h>

#undef	ntohl

unsigned long int
DEFUN(ntohl, (x), unsigned long int x)
{
  return x;
}
