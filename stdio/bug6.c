#include <ansidecl.h>
#include <stdio.h>

int
DEFUN_VOID(main)
{
  char buf[80];
  int i;
  FILE *f = fopen ("scanf.dat", "r");
  if (f == NULL) perror ("");
  fscanf (f, "%2s", buf);
  if (buf[0] != 'X' || buf[1] != 'Y' || buf[2] != '\0')
    printf ("test of %%2s failed.\n");
  fscanf (f, " ", buf);
  fscanf (f, "%d", &i);
  if (i != 1234) printf ("test of %%d failed.\n");
  fscanf (f, "%c", buf);
  if (buf[0] != 'L') printf ("test of %%c failed.\n");
  return 0;
}
