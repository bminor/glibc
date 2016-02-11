#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* Build like this:

   gcc -shared -fpic mtrace-ctl.c -o /tmp/mtrace-ctl.so ../../glibc.build/libc.so

   Invoke like this:

   LD_PRELOAD=/tmp/mtrace-ctl.so ./myprog

*/

#include "mtrace.h"

static void
err(const char *str)
{
  write (2, str, strlen(str));
  write (2, "\n", 1);
  exit(1);
}

void __attribute__((constructor))
djmain()
{
  char *e;
  int sz;

  e = getenv("MTRACE_CTL_COUNT");
  if (!e)
    e = "1000";
  sz = atoi(e) * sizeof(struct __malloc_trace_buffer_s);

  char *buf = mmap (NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (!buf)
    err("Cannot mmap");

  buf[0] = 1;
  buf[sz-1] = 1;

  /* This must be the last thing we do.  */
  __malloc_set_trace_buffer ((void *)buf, sz);
  return;
}

const char * const typenames[] = {
  "unused  ",
  "malloc  ",
  "calloc  ",
  "free    ",
  "realloc ",
  "memalign",
  "valloc  ",
};

void __attribute__((destructor))
djend()
{
  char *e;
  FILE *outf;
  int head, size, i;

  /* Prevent problems with recursion etc by shutting off trace right away.  */
  __malloc_trace_buffer_ptr buf = __malloc_get_trace_buffer (&size, &head);
  __malloc_set_trace_buffer (NULL, 0);

  e = getenv("MTRACE_CTL_FILE");
  if (!e)
    e = "/tmp/mtrace.out";

  outf = fopen(e, "w");
  if (!outf)
    err("cannot open output file");
  setbuf (outf, NULL);

  fprintf (outf, "%d out of %d events captured\n", head, size);

  fprintf (outf, "threadid type     path    ptr1             size             ptr2\n");
  for (i=0; i<size; i++)
    {
      __malloc_trace_buffer_ptr t = buf + (i+head) % size;

      switch (t->type)
	{
	case __MTB_TYPE_UNUSED:
	  break;
	default:
	  fprintf (outf, "%08x %s %d%d%d%d%d%d%d %016x %016x %016x\n",
		   t->thread,
		   typenames[t->type],
		   t->path_thread_cache,
		   t->path_cpu_cache,
		   t->path_cpu_cache2,
		   t->path_sbrk,
		   t->path_mmap,
		   t->path_munmap,
		   t->path_m_f_realloc,
		   t->ptr1,
		   t->size,
		   t->ptr2);
	  break;
	}
    }
  fclose (outf);

  munmap (buf, size * sizeof(struct __malloc_trace_buffer_s));
  return;
}
