#include <ansidecl.h>
#include <netinet/in.h>

#undef	htons

unsigned short int
DEFUN(htons, (x), unsigned short int x)
{
  return x;
}
