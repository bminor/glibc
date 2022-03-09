#include <link.h>
#include <stdbool.h>
#include <stdio.h>

static int o, x;

#define ELEMS O O O O O O O O X X X X X X X O O X O O X X X E X E E O X O E
#define E 0,

#define O &o,
#define X &x,
void *arr[] = { ELEMS };
#undef O
#undef X

#define O 1,
#define X 2,
static char val[] = { ELEMS };

static int
do_test (void)
{
  ElfW(Dyn) *d = _DYNAMIC;
  if (d)
    {
      bool has_relr = false;
      for (; d->d_tag != DT_NULL; d++)
	if (d->d_tag == DT_RELR)
	  has_relr = true;
      if (!has_relr)
	{
	  fprintf (stderr, "no DT_RELR\n");
	  return 1;
	}
    }

  for (int i = 0; i < sizeof (arr) / sizeof (arr[0]); i++)
    if (!((arr[i] == 0 && val[i] == 0) ||
	  (arr[i] == &o && val[i] == 1) ||
	  (arr[i] == &x && val[i] == 2)))
      return 1;
  return 0;
}

#include <support/test-driver.c>
