#include <ansidecl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int
DEFUN(compare, (a, b), CONST PTR a AND CONST PTR b)
{
  return strcmp (*(char **) a, *(char **) b);
}

int
DEFUN_VOID(main)
{
  static char *lines[500];
  static size_t lens[500];
  size_t i;

  i = 0;
  while (i < 500 && getline (&lines[i], &lens[500], stdin) > 0)
    ++i;
  if (i < 500)
    lines[i] = NULL;

  while (--i > 0)
    {
      size_t swap = random () % i;
      char *line = lines[swap];
      lines[swap] = lines[i];
      lines[i] = line;
    }

  qsort (lines, 500, sizeof (char *), compare);

  for (i = 0; i < 500 && lines[i] != NULL; ++i)
    fputs (lines[i], stdout);

  return 0;
}
