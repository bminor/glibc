/* If stdio is working correctly, after this is run infile and outfile
   will have the same contents.  If the bug (found in GNU C library 0.3)
   exhibits itself, outfile will be missing the 2nd through 1023rd
   characters.  */

#include <ansidecl.h>
#include <stdio.h>
#include <stdlib.h>

static char buf[8192];

int
main (int argc, char **argv)
{
  FILE *in;
  FILE *out;
  static char inname[] = "infile";
  static char outname[] = "outfile";
  int i;

  /* Create a test file.  */
  in = fopen (inname, "w+");
  if (in == NULL) perror (inname);
  for (i = 0; i < 1000; ++i)
    fprintf (in, "%d\n", i);

  out = fopen (outname, "w");
  if (out == NULL) perror (outname);
  if (fseek (in, 0L, SEEK_SET) != 0) abort ();
  putc (getc (in), out);
  i = fread (buf, 1, sizeof (buf), in);
  if (i == 0) perror (inname);
  if (fwrite (buf, 1, i, out) != i) perror (outname);
  fclose (in);
  fclose (out);

  execlp ("cmp", "cmp", inname, outname, (char *) NULL);
  perror ("execlp: cmp");
  exit (1);
}
