#include <ansidecl.h>
#include <stdio.h>

int
DEFUN_VOID(main)
{
  char buf[80];
  int i;
  int lost = 0;

  scanf ("%2s", buf);
  if (lost |= (buf[0] != 'X' || buf[1] != 'Y' || buf[2] != '\0'))
    puts ("test of %2s failed.");
  scanf (" ");
  scanf ("%d", &i);
  if (lost |= (i != 1234))
    puts ("test of %d failed.");
  scanf ("%c", buf);
  if (lost |= (buf[0] != 'L'))
    puts ("test of %c failed.\n");

  puts (lost ? "Test FAILED!" : "Test succeeded.");
  return lost;
}
