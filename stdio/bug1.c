#include <ansidecl.h>
#include <stdio.h>

void main (void)
{
  char *bp;
  size_t size;
  FILE *stream;

  stream = open_memstream (&bp, &size);
  fprintf (stream, "hello");
  fflush (stream);
  printf ("buf = %s, size = %d\n", bp, size);
  fprintf (stream, ", world");
  fclose (stream);
  printf ("buf = %s, size = %d\n", bp, size);
}
  
