#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

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

#define estr(str) write (2, str, strlen (str))

#if 0
static void
err(const char *str)
{
  estr (str);
  estr ("\n");
}
#endif

/*
 * mtrace_start - checks for buffer, allocates one if needed, starts trace.
 * mtrace_stop - stops tracing
 * mtrace_sync - syncs the buffer
 * mtrace_reset - resets buffer state to intial state
 */

struct _malloc_trace_buffer_s *mtrace_buffer = NULL;
size_t mtrace_buffer_bytesize = 0;

int
mtrace_start (void)
{
  const char *e;
  char *fname;
  int sequence = 0;

  e = getenv("MTRACE_CTL_COUNT");
  if (!e)
    e = "1000";

  e = getenv("MTRACE_CTL_FILE");
  if (!e)
    e = "/tmp/mtrace.out";

  fname = alloca (strlen(e) + 30);
  sprintf(fname, "%s.%d", e, getpid());
  while (access (fname, F_OK) == 0)
    {
      sequence ++;
      sprintf(fname, "%s.%d.%d", e, getpid(), sequence);
    }

  estr ("mtrace-ctl: writing to ");
  estr (fname);
  estr ("\n");

  __malloc_trace_init (fname);
  return 0;
}

void
mtrace_stop (void)
{
  size_t count;
  char line[100];

  count = __malloc_trace_stop ();
  sprintf (line, "mtrace-ctl: %lld entries recorded\n", (long long)count);
  estr (line);
}

void
mtrace_sync (void)
{
  __malloc_trace_sync ();
  //  __malloc_trace_buffer_ptr buf = __malloc_get_trace_buffer (&size, &head);
  //  msync (buf, size * sizeof(struct __malloc_trace_buffer_s), MS_SYNC | MS_INVALIDATE);
}

void
mtrace_reset (void)
{

  __malloc_trace_stop ();
  mtrace_start ();
}

void __attribute__((constructor))
mtrace_ctor(void)
{
  if (mtrace_start ())
    exit (1);
}

void __attribute__((destructor))
mtrace_dtor(void)
{
  mtrace_stop ();
  mtrace_sync ();
}

#if 0

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
#endif
