#include <ansidecl.h>
#include <stdio.h>
#include <stdlib.h>

int
DEFUN_VOID(main)
{
  wchar_t w[10];
  char c[10];
  int i;
  int lose = 0;

  i = mbstowcs (w, "bar", 4);
  if (!(i == 3 && w[1] == 'a'))
    {
      puts ("mbstowcs FAILED!");
      lose = 1;
    }

  mbstowcs (w, "blah", 5);
  i = wcstombs (c, w, 10);
  if (i != 4)
    {
      puts ("wcstombs FAILED!");
      lose = 1;
    }

  if (mblen ("foobar", 7) != -1)
    {
      puts ("mblen 1 FAILED!");
      lose = 1;
    }

  if (mblen ("", 1) != 0)
    {
      puts ("mblen 2 FAILED!");
      lose = 1;
    }

  puts (lose ? "Test FAILED!" : "Test succeeded.");
  return lose;
}
