#include <ansidecl.h>
#include <stdio.h>

void write_data (FILE *stream)
{
  int i;
  for (i=0; i<100; i++)
    fprintf (stream, "%d\n", i);
  if (ferror (stream))  {
    fprintf (stderr, "Output to stream failed.\n");
    exit (1);
    }
}

void main (void)
{
  FILE *output;
  int status;

  output = popen ("/usr/ucb/more", "w");
  if (!output) {
    fprintf (stderr, "Could not run more.\n");
    exit (1);
    }
  write_data (output);
  status = pclose (output);
  fprintf (stderr, "pclose returned %d\n", status);
  exit (0);
}

  

  
