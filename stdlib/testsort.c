#include <ansidecl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int
DEFUN_VOID(main)
{
  static char *lines[500];
  static size_t lens[500];
  size_t i;

  i = 0;
  while (i < 500 && getline (&lines[i], &lens[500]) > 0)
    ++i;
  if (i < 500)
    lines[i] = NULL;

  qsort (lines, 500, sizeof (char *), strcmp);

  while (i < 500 && lines[i] != NULL)
    fputs (lines[i], stdout);

  return 0;
}
