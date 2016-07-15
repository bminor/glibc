#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unordered_map>

// The trace file looks like an array of struct __malloc_trace_buffer_s
#include "mtrace.h"

// This application is "run once and exit" so there's no cleanup code.

typedef unsigned char byte;

struct __malloc_trace_buffer_s *trace_records;
size_t num_trace_records;

int verbose = 0;

//------------------------------------------------------------
// File data buffers

#define BUFFER_SIZE 4096

struct BufferBlock {
  BufferBlock *next;
  byte buf[BUFFER_SIZE];
};

struct Buffer {
  BufferBlock *first_buffer;
  BufferBlock *last_buffer;

  int count_total;
  int count_last;

  Buffer();
  void add (char x);
  void add_int (size_t i);
  void write (int fd);
  void clear (void);
};

Buffer::Buffer()
{
  first_buffer = last_buffer = new BufferBlock();
  count_total = count_last = 0;
}

void
Buffer::add (char x)
{
  if (count_last == BUFFER_SIZE)
    {
      BufferBlock *b = new BufferBlock;
      last_buffer->next = b;
      last_buffer = b;
      count_last = 0;
    }
  last_buffer->buf[count_last] = x;
  count_last ++;
  count_total ++;
}

void
Buffer::add_int (size_t val)
{
  byte buf[sizeof(size_t)*2];
  int i = 0;

  buf[i++] = val & 127;
  while (val > 127)
    {
      val >>= 7;
      buf[i++] = (val & 127) | 128;
    }
  while (i > 0)
    add (buf[--i]);
}

void
Buffer::write (int fd)
{
  BufferBlock *b;
  for (b = first_buffer; b != last_buffer; b = b->next)
    ::write (fd, b->buf, BUFFER_SIZE);
  if (count_last)
    ::write (fd, last_buffer->buf, count_last);
}

void
Buffer::clear (void)
{
  while (first_buffer != last_buffer)
    {
      BufferBlock *b = first_buffer->next;
      delete first_buffer;
      first_buffer = b;
    }
  count_total = count_last = 0;
}

//------------------------------------------------------------

struct PerThread {
  int started;
  Buffer workload;
  PerThread() : started(0), workload() {};
  void add (byte x) { workload.add(x); }
  void add_int (size_t x) { workload.add_int(x); }
};

typedef std::unordered_map<int32_t, PerThread*> PerThreadMap;
PerThreadMap per_thread;

struct PerAddr {
  PerThread *owner;
  void *ptr;
  int idx;
  int valid;
  const char *reason;
  int reason_idx;
  PerAddr(void *_ptr) : owner(0), ptr(_ptr), valid(0), reason("not seen") {};
};

// Don't start at zero, zero is special.
int addr_count = 1;

std::unordered_map<void *, PerAddr*> per_addr;

PerAddr *
get_addr (void *ptr)
{
  PerAddr *p;
  if (ptr == NULL)
    return NULL;
  p = per_addr[ptr];
  if (!p)
    {
      p = per_addr[ptr] = new PerAddr(ptr);
      p->idx = addr_count ++;
    }
  return p;
}

int sync_counter = 0;

// Insert a release/acquire pair to transfer ownership of data
// from thread TREL to thread TACK
void
sync_threads (PerThread *trel, PerThread *tacq)
{
  if (trel == tacq)
    return;
  sync_counter ++;
  trel->add (C_SYNC_W);
  trel->add_int (sync_counter);
  tacq->add (C_SYNC_R);
  tacq->add_int (sync_counter);
}

void
acq_ptr (PerThread *thread, PerAddr *addr)
{
  if (addr == NULL)
    return;
  if (addr->owner != NULL && addr->owner != thread)
    sync_threads (addr->owner, thread);
  addr->owner = thread;
}

//------------------------------------------------------------

int
main(int argc, char **argv)
{
  int trace_fd, wl_fd;
  struct stat stbuf;

  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    {
      verbose ++;
      argc --;
      argv ++;
    }

  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s <outputfile.wl> <inputfile.mtrace>\n", argv[0]);
      exit(1);
    }

  if (access (argv[1], F_OK) == 0)
    {
      fprintf (stderr, "Error: output file %s already exists, will not overwrite\n", argv[1]);
      exit(1);
    }

  trace_fd = open (argv[2], O_RDONLY, 0666);
  if (trace_fd < 0)
    {
      fprintf (stderr, "Can't open %s for reading\n", argv[2]);
      perror("The error was");
      exit(1);
    }

  if (stat (argv[2], &stbuf) < 0)
    {
      fprintf (stderr, "Can't stat %s for reading\n", argv[2]);
      perror("The error was");
      exit(1);
    }

  trace_records =
    (struct __malloc_trace_buffer_s *)
    mmap (NULL, stbuf.st_size, PROT_READ, MAP_SHARED, trace_fd, 0);
  if (trace_records == (void *)(-1))
    {
      fprintf (stderr, "Can't map %s for reading\n", argv[2]);
      perror("The error was");
      exit(1);
    }
  num_trace_records = stbuf.st_size / sizeof(*trace_records);

  PerThread *thread = NULL;
  int last_tid = -1;
  PerThread *master_thread = NULL;

  per_addr[0] = NULL;

  for (unsigned int i = 0; i < num_trace_records; i++)
    {
      __malloc_trace_buffer_s *r = trace_records + i;

      // Quick-skip for NULs at EOF
      if (r->type == __MTB_TYPE_UNUSED)
	continue;

      if(verbose)
	printf("\033[32m%8x %2x (0x%p, 0x%x) =  0x%p\033[0m\n",
	       r->thread, r->type, r->ptr1, (int)r->size, r->ptr2);

      if (r->thread != last_tid)
	{
	  thread = per_thread[r->thread];
	  if (thread == NULL)
	    thread = per_thread[r->thread] = new PerThread();
	  last_tid = r->thread;
	}
      if (!master_thread)
	{
	  master_thread = thread;
	  thread->started = 1;
	}
      else if (!thread->started)
	{
	  sync_threads (master_thread, thread);
	  thread->started = 1;
	}


      PerAddr *pa1 = get_addr(r->ptr1);
      PerAddr *pa2 = get_addr(r->ptr2);

      switch (r->type)
	{
	case __MTB_TYPE_UNUSED:
	case __MTB_TYPE_MAGIC:
	  break;

	case __MTB_TYPE_MALLOC:
	case __MTB_TYPE_CALLOC:
	  acq_ptr (thread, pa2);
	  if (pa2 && pa2->valid)
	    printf ("%d: pointer %p malloc'd again?  %d:%s\n", i, pa2->ptr, pa2->reason_idx, pa2->reason);
	  thread->add (r->type == __MTB_TYPE_MALLOC ? C_MALLOC : C_CALLOC);
	  thread->add_int (pa2 ? pa2->idx : 0);
	  thread->add_int (r->size);
	  if (pa2)
	    {
	      pa2->valid = 1;
	      pa2->reason = "malloc";
	      pa2->reason_idx = i;
	    }
	  break;

	case __MTB_TYPE_FREE:
	  acq_ptr (thread, pa1);
	  if (pa1 == NULL)
	    {
	      thread->add (C_FREE);
	      thread->add_int (0);
	    }
	  else if (pa1->valid)
	    {
	      thread->add (C_FREE);
	      thread->add_int (pa1->idx);
	      pa1->valid = 0;
	      pa1->reason = "previously free'd";
	      pa1->reason_idx = i;
	    }
	  else
	    {
	      printf("%d: invalid pointer %p passed to free: %d:%s\n", i, pa1->ptr, pa1->reason_idx, pa1->reason);
	    }
	  break;

	case __MTB_TYPE_REALLOC:
	  if (pa1 && pa1->owner)
	    acq_ptr (thread, pa1);
	  if (pa2 && pa2->owner)
	    acq_ptr (thread, pa2);
	  thread->add (C_REALLOC);
	  thread->add_int (pa2 ? pa2->idx : 0);
	  thread->add_int (pa1 ? pa1->idx : 0);
	  thread->add_int (r->size);

	  if (pa1)
	    {
	      pa1->valid = 0;
	      pa1->reason = "previously realloc'd";
	      pa1->reason_idx = i;
	    }
	  if (pa2)
	    {
	      pa2->valid = 1;
	      pa2->reason = "realloc";
	      pa2->reason_idx = i;
	    }

	  break;
	}
    }

  int n_threads = per_thread.size();
  PerThread *threads[n_threads];
  int thread_off[n_threads];
  int i = 0;

  PerThreadMap::iterator iter;
  if(verbose)
    printf("%d threads\n", (int)per_thread.size());
  for (iter = per_thread.begin();
       iter != per_thread.end();
       ++iter)
    {
      threads[i++] = iter->second;
      iter->second->add(C_DONE);
      if(verbose)
	printf("thread: %d bytes\n", iter->second->workload.count_total);
    }

  /* The location of each thread's workload depends on the size of the
     startup block, but the size of the startup block depends on the
     size of the thread's location encoding.  So, we loop until it
     stabilizes.  */
  int old_len = 1;
  int new_len = 2;
  Buffer main_loop;
  while (old_len != new_len)
    {
      int off = new_len;
      int i;

      old_len = new_len;
      main_loop.clear ();

      main_loop.add (C_ALLOC_PTRS);
      main_loop.add_int (addr_count);
      main_loop.add (C_ALLOC_SYNCS);
      main_loop.add_int (sync_counter);
      main_loop.add (C_NTHREADS);
      main_loop.add_int (n_threads);

      for (i=0; i<n_threads; i++)
	{
	  thread_off[i] = off;
	  main_loop.add (C_START_THREAD);
	  main_loop.add_int (off);
	  off += threads[i]->workload.count_total;
	}

      main_loop.add (C_DONE);

      new_len = main_loop.count_total;
    }

  wl_fd = open (argv[1], O_CREAT|O_EXCL|O_RDWR, 0666);
  if (wl_fd < 0)
    {
      fprintf (stderr, "Can't open %s for writing\n", argv[1]);
      perror("The error was");
      exit(1);
    }

  main_loop.write (wl_fd);

  for (i=0; i<n_threads; i++)
    {
      if (verbose)
	printf("Start thread[%d] offset 0x%x\n", i, thread_off[i]);
      threads[i]->workload.write (wl_fd);
    }

  close (wl_fd);

  return 0;
}
