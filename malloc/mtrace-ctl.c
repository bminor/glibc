#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* This module is a stand-alone control program for malloc's internal
   trace buffer.  It is intended to be preloaded like this:

   LD_PRELOAD=/usr/lib/libmtracectl.so ./myprog

   This module uses the following environment variables:

   MTRACE_CTL_COUNT - how many records to store (default: 1000).  Each
   record is 32 bytes, and the entire buffer is mmap'd at once.  If
   the buffer isn't big enough, it will overwrite early records with
   newer ones.  The total number of trace records is reported in the
   output file so that a larger buffer may be allocated on future runs.

   MTRACE_CTL_FILE - the output file name (default:
   /tmp/mtrace-$$.out).  Note that the default is per-pid but there is
   no way to specify a per-pid pattern via this environment variable.

   The output file will contain a header that says how many trace
   records were seen (which is usually more or less than the trace
   buffer size).  The trace buffer is then dumped one entry per line.

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
djmain(void)
{
  const char *e;
  size_t sz;

  e = getenv("MTRACE_CTL_COUNT");
  if (!e)
    e = "1000";
  sz = (size_t) atol(e) * sizeof(struct __malloc_trace_buffer_s);

  char *buf = mmap (NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (buf == NULL || buf == (char *)(-1))
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
  "pvalloc  ",
};

void __attribute__((destructor))
djend(void)
{
  char *e;
  FILE *outf;
  size_t head, size, i;

  /* Prevent problems with recursion etc by shutting off trace right away.  */
  __malloc_trace_buffer_ptr buf = __malloc_get_trace_buffer (&size, &head);
  __malloc_set_trace_buffer (NULL, 0);

  e = getenv("MTRACE_CTL_FILE");
  if (!e)
    {
      static char fname[100];
      sprintf(fname, "/tmp/mtrace-%d.out", getpid());
      e = fname;
    }

  outf = fopen(e, "w");
  if (!outf)
    err("cannot open output file");
  setbuf (outf, NULL);

  fprintf (outf, "%ld out of %ld events captured\n", (long)head, (long)size);

  fprintf (outf, "threadid type     path     ptr1             size             ptr2\n");
  for (i=0; i<size; i++)
    {
      __malloc_trace_buffer_ptr t = buf + (i+head) % size;

      switch (t->type)
	{
	case __MTB_TYPE_UNUSED:
	  break;
	default:
	  fprintf (outf, "%08x %s %c%c%c%c%c%c%c%c %016llx %016llx %016llx\n",
		   t->thread,
		   typenames[t->type],
		   t->path_thread_cache ? 'T' : '-',
		   t->path_cpu_cache ? 'c' : '-',
		   t->path_cpu_cache2 ? 'C' : '-',
		   t->path_sbrk ? 's' : '-',
		   t->path_mmap ? 'M' : '-',
		   t->path_munmap ? 'U' : '-',
		   t->path_m_f_realloc ? 'R' : '-',
		   t->path_hook ? 'H' : '-',
		   (long long unsigned int) (size_t) t->ptr1,
		   (long long unsigned int) t->size,
		   (long long unsigned int) (size_t) t->ptr2);
	  break;
	}
    }
  fclose (outf);

  munmap (buf, size * sizeof(struct __malloc_trace_buffer_s));
  return;
}
