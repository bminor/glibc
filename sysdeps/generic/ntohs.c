#include <ansidecl.h>
#include <netinet/in.h>

#undef	ntohs

unsigned short int
DEFUN(ntohs, (x), unsigned short int x)
{
  return x;
}
