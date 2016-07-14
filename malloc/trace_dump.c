#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// The trace file looks like an array of struct __malloc_trace_buffer_s
#include "mtrace.h"

static size_t
get_int (unsigned char **ptr)
{
  size_t rv = 0;
  while (1)
  {
    unsigned char c = *(*ptr)++;
    rv |= (c & 0x7f);
    if (c & 0x80)
      rv <<= 7;
    else
      return rv;
  }
}


int
data_looks_like_raw_trace (unsigned char *data, long n_data)
{
  long lim = n_data > 1024 ? 1020 : (n_data - 4);
  long i;

  // free and malloc calls will have a NULL we can look for
  for (i=0; i<lim; i++)
    if (memcmp (data+i, "\0\0\0\0", 4) == 0)
	return 1;

  return 0;
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

void
dump_raw_trace (unsigned char *data, long n_data)
{
  unsigned char *edata = data + n_data;
  long head;

  head = n_data / sizeof (struct __malloc_trace_buffer_s);

  printf ("%ld out of %ld events captured (I think)\n", head, head);

  printf ("threadid type     path     ptr1             size             ptr2\n");

  while (data <= edata - sizeof (struct __malloc_trace_buffer_s))
    {
      struct __malloc_trace_buffer_s *t = (struct __malloc_trace_buffer_s *)data;

      switch (t->type)
	{
	case __MTB_TYPE_UNUSED:
	  break;
	default:
	  printf ("%08x %s %c%c%c%c%c%c%c%c %016llx %016llx %016llx\n",
		  t->thread,
		  t->type == __MTB_TYPE_MAGIC ? "magic   " : typenames[t->type],
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

      data += sizeof (struct __malloc_trace_buffer_s);
    }
}

void
dump_workload (unsigned char *data, long n_data)
{
  unsigned char *orig_data = data;
  unsigned char *edata = data + n_data;
  int thread_idx = 0;
  int n_ptrs, n_syncs, n_threads, idx, p1, p2, sz;

  while (data < edata)
    {
      printf("%016lx: %4d: ", data - orig_data, thread_idx);
      switch (*data++)
	{
	case C_NOP:
	  break;
	case C_ALLOC_PTRS:
	  n_ptrs = get_int(&data);
	  printf("AllocPtrs: %d\n", n_ptrs);
	  break;
	case C_ALLOC_SYNCS:
	  n_syncs = get_int(&data);
	  printf("AllocSyncs: %d\n", n_syncs);
	  break;
	case C_NTHREADS:
	  n_threads = get_int (&data);
	  printf("NThreads: %d\n", n_threads);
	  break;
	case C_START_THREAD:
	  idx = get_int (&data);
	  printf("StartThread: 0x%x\n", idx);
	  break;
	case C_DONE:
	  printf("Done\n");
	  thread_idx ++;
	  break;

	case C_MALLOC:
	  p2 = get_int (&data);
	  sz = get_int (&data);
	  printf("Malloc (%d) -> %d\n", sz, p2);
	  break;

	case C_CALLOC:
	  p2 = get_int (&data);
	  sz = get_int (&data);
	  printf("Calloc (%d) -> %d\n", sz, p2);
	  break;

	case C_REALLOC:
	  p2 = get_int (&data);
	  p1 = get_int (&data);
	  sz = get_int (&data);
	  printf("Realloc (%d, %d) -> %d\n", p1, sz, p2);
	  break;

	case C_FREE:
	  p1 = get_int (&data);
	  printf("Free (%d)\n", p1);
	  break;

	case C_SYNC_W:
	  p1 = get_int(&data);
	  printf("SyncW (%d)\n", p1);
	  break;

	case C_SYNC_R:
	  p1 = get_int(&data);
	  printf("SyncR (%d)\n", p1);
	  break;

	default:
	  printf("(unknown:%d)\n", *--data);
	  exit(1);
	}
    }
}

int
main (int argc, char **argv)
{
  int fd;
  struct stat statb;
  unsigned char *data;

  fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    {
      fprintf(stderr, "Unable to open %s for reading\n", argv[1]);
      perror("The error was");
      exit(1);
    }
  fstat (fd, &statb);

  data = (unsigned char *) mmap (NULL, statb.st_size, PROT_READ, MAP_SHARED, fd, 0);

  if (data_looks_like_raw_trace (data, statb.st_size))
    dump_raw_trace (data, statb.st_size);
  else
    dump_workload (data, statb.st_size);

  return 0;
}
