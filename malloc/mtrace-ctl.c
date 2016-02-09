#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "mtrace.h"

void __attribute__((constructor))
djmain()
{
  char *e = getenv("MTRACE_CTL");
  if (!e) e = "1000";
  int sz = atoi(e) * sizeof(struct __malloc_trace_buffer_s);
  char *buf = sbrk (sz+15);
  while ((intptr_t)buf & 15)
    buf ++;
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
  FILE *outf;
  int head, size, i;
  __malloc_trace_buffer_ptr buf = __malloc_get_trace_buffer (&size, &head);
  outf = fopen("/tmp/mtrace.out", "w");
  if (!outf)
    return;

  fprintf (outf, "threadid type     path    ptr1             size             ptr2\n");
  for (i=0; i<size; i++)
    {
      __malloc_trace_buffer_ptr t = buf + i % size;

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
  return;
}
