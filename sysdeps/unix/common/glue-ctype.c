#include <ctype.h>

main ()
{
  int i;

  puts ("#include <ansidecl.h>");
  puts ("CONST char _ctype_[] =");
  puts ("  {");

  for (i = -1; i < 256; ++i)
    printf ("    %u,\n", (unsigned int) ((_ctype_+1)[i]));

  puts ("  };");

  exit (0);
}


