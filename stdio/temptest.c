#include <ansidecl.h>
#include <stdio.h>

main ()
{
  char *fn;
  FILE *fp;
  int i;

  for (i = 0; i < 500; i++) {
    fn = __stdio_gen_tempname((CONST char *) NULL,
	"file", 0, (size_t *) NULL);
    if (fn == NULL) {
      printf ("__stdio_gen_tempname failed\n");
      exit (1);
    }
    printf ("file: %s\n", fn);
    fp = fopen (fn, "w");
    fclose (fp);
  }
}
