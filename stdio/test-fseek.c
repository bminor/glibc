#include <ansidecl.h>
#include <stdio.h>

#define TESTFILE "test.dat"

int
main ()
{
  FILE *fp;
  int i, j;

  puts ("\nFile seek test");
  if ((fp = fopen (TESTFILE, "w")) != NULL)
    {
      for (i = 0; i < 256; i++)
	putc (i, fp);
      if (freopen (TESTFILE, "r", fp) != fp)
	puts ("Cannot open file for reading");
      else
	{
	  for (i = 1; i <= 255; i++)
	    {
	      printf ("\r%3d ", i);
	      fflush (stdout);
	      fseek (fp, (long) -i, SEEK_END);
	      if ((j = getc (fp)) != 256 - i)
		{
		  printf ("SEEK_END failed %d\n", j);
		  break;
		}
	      if (fseek (fp, (long) i, SEEK_SET))
		{
		  puts ("Cannot SEEK_SET");
		  break;
		}
	      if ((j = getc (fp)) != i)
		{
		  printf ("SEEK_SET failed %d\n", j);
		  break;
		}
	      if (fseek (fp, (long) i, SEEK_SET))
		{
		  puts ("Cannot SEEK_SET");
		  break;
		}
	      if (fseek (fp, (long) (i >= 128 ? -128 : 128), SEEK_CUR))
		{
		  puts ("Cannot SEEK_CUR");
		  break;
		}
	      if ((j = getc (fp)) != (i >= 128 ? i - 128 : i + 128))
		{
		  printf ("SEEK_CUR failed %d\n", j);
		  break;
		}
	    }
	  fclose (fp);
	  if (i > 255)
	    puts ("ok");
	}
    }
}
