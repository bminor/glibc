#include <ansidecl.h>
#include <stdio.h>
#include <stdlib.h>

void
DEFUN(write_data, (stream), FILE *stream)
{
  int i;
  for (i=0; i<100; i++)
    fprintf (stream, "%d\n", i);
  if (ferror (stream))  {
    fprintf (stderr, "Output to stream failed.\n");
    exit (1);
    }
}

void
DEFUN(read_data, (stream), FILE *stream)
{
  int i, j;

  for (i=0; i<100; i++)
    {
      if (fscanf (stream, "%d\n", &j) != 1 || j != i)
	{
	  if (ferror (stream))
	    perror ("fscanf");
	  puts ("Test FAILED!");
	  exit (1);
	}
    }
}

int
DEFUN_VOID(main)
{
  FILE *output, *input;
  int status;

  output = popen ("/bin/cat >tstpopen.tmp", "w");
  if (output == NULL)
    {
      perror ("popen");
      puts ("Test FAILED!");
      exit (1);
    }
  write_data (output);
  status = pclose (output);
  fprintf (stderr, "pclose returned %d\n", status);
  input = fopen ("tstpopen.tmp", "r");
  if (input == NULL)
    {
      perror ("tstpopen.tmp");
      puts ("Test FAILED!");
      exit (1);
    }
  read_data (input);
  (void) fclose (input);

  puts (status ? "Test FAILED!" : "Test succeeded.");
  exit (status);
}

  

  
