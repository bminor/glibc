#include <ansidecl.h>
#include <stdio.h>
#include <stdlib.h>

int
main ()
{
  wchar_t foo[5];
  int i;
  int lose;

  i = mbstowcs (foo, "bar", 4);
  lose = (i == 3 && foo[1] == 'a');

  puts (lose ? "Test FAILED!" : "Test succeeded.");
  return lose;
}
