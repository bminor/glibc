#include <ansidecl.h>
#include <stdio.h>

int
main ()
{
  FILE *f;
  int i;

  f = fopen("bugtest", "w+");
  for (i=0; i<9000; i++) {
    putc('x', f);
  }
  fseek(f, 8180L, 0);
  fwrite("Where does this text go?", 1, 24, f);
  fclose(f);
  return 0;
}
