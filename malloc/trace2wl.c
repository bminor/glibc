#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <getopt.h>
#include <time.h>

/* The trace file looks like an array of struct __malloc_trace_buffer_s */
#include "mtrace.h"

/* This application is "run once and exit" so there's no cleanup code. */

typedef unsigned char byte;

struct __malloc_trace_buffer_s *trace_records;
size_t num_trace_records;

int verbose = 0;
int use_file_buffers = 0;

//------------------------------------------------------------
// File data buffers

static int tmpfile_idx = 0;
static char *tmpdir;
static int tmpdir_len;

#define BUFFER_SIZE 4096

/* If we're using memory buffers, we chain from first_buffer to
   last_buffer as a linked list.  If we're using disk buffers, we only
   use last_buffer, which points to a fixed buffer.  */

typedef struct BufferBlock {
  struct BufferBlock *next;
  byte buf[BUFFER_SIZE];
} BufferBlock;

typedef struct Buffer {
  char *filename;
  int fd;
  BufferBlock *first_buffer;
  BufferBlock *last_buffer;

  size_t count_total;
  size_t count_last;
} Buffer;

void
Buffer__ctor(Buffer *this)
{
  if (use_file_buffers)
    {
      this->filename = (char *) malloc (tmpdir_len + 7);
      sprintf (this->filename, "%s%06d", tmpdir, tmpfile_idx);
      tmpfile_idx ++;
      this->fd = -1;
    }
  this->first_buffer = this->last_buffer = (BufferBlock *) malloc (sizeof(BufferBlock));
  this->first_buffer->next = NULL;
  this->count_total = this->count_last = 0;
}

void
Buffer__add (Buffer *this, char x)
{
  if (this->count_last == BUFFER_SIZE)
    {
      if (use_file_buffers)
	{
	  if (this->fd == -1)
	    {
	      this->fd = open(this->filename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	      if (this->fd < 0)
		{
		  fprintf(stderr, "cannot create temporary file %s for writing\n", this->filename);
		  perror("The error was");
		  exit(1);
		}
	    }
	  write (this->fd, this->last_buffer->buf, BUFFER_SIZE);
	}
      else
	{
	  BufferBlock *b = (BufferBlock *) malloc (sizeof(BufferBlock));
	  b->next = NULL;
	  this->last_buffer->next = b;
	  this->last_buffer = b;
	}
      this->count_last = 0;
    }
  this->last_buffer->buf[this->count_last] = x;
  this->count_last ++;
  this->count_total ++;
}

void
Buffer__add_int (Buffer *this, size_t val)
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
    Buffer__add (this, buf[--i]);
}

void
Buffer__write (Buffer *this, int fd)
{
  if (use_file_buffers)
    {
      byte buf[BUFFER_SIZE];
      int count;
      struct stat s;

      if (this->fd != -1)
	close (this->fd);

      if (this->count_total != this->count_last)
	{
	  if (stat(this->filename, &s) < 0)
	    {
	      fprintf(stderr, "Cannot stat %s\n", this->filename);
	      perror("The error was");
	      exit(1);
	    }
	  if (s.st_size != this->count_total - this->count_last)
	    {
	      fprintf(stderr, "File %s is %ld, not %ld-%ld !\n", this->filename, s.st_size, this->count_total, this->count_last);
	      exit(1);
	    }

	  this->fd = open (this->filename, O_RDONLY);
	  while ((count = read (this->fd, buf, BUFFER_SIZE)) > 0)
	    write (fd, buf, count);
	  unlink (this->filename);
	}

      this->fd = -1;
    }
  else
    {
      BufferBlock *b;

      for (b = this->first_buffer; b != this->last_buffer; b = b->next)
	write (fd, b->buf, BUFFER_SIZE);
    }

  if (this->count_last)
    write (fd, this->last_buffer->buf, this->count_last);
  this->count_last = 0;
}

void
Buffer__clear (Buffer *this)
{
  if (use_file_buffers)
    {
      if (this->fd != -1)
	close (this->fd);
      unlink (this->filename);
      this->fd = -1;
    }
  else
    while (this->first_buffer != this->last_buffer)
      {
	BufferBlock *b = this->first_buffer->next;
	free (this->first_buffer);
	this->first_buffer = b;
      }
  this->count_total = this->count_last = 0;
}

//------------------------------------------------------------

typedef struct Hash {
  /* Each page table is an array of pointers to page tables... */
  void *pt[256];
  int count;
} Hash;

void **
Hash__hash (Hash *this, size_t key)
{
  int i, pi;
  void **pt;

  if (sizeof (size_t) >= 8)
    key >>= 3;
  else
    key >>= 2;

  pt = this->pt;
  for (i=sizeof(size_t)-1; i>0; i--)
    {
      pi = (key >> (i*8)) & 0xff;
      if (pt[pi] == NULL)
	pt[pi] = (void **) calloc (256, sizeof(void *));
      pt = pt[pi];
    }

  pi = key & 0xff;
  return &(pt[pi]);
}

void
Hash__ctor (Hash *this)
{
  memset (&this->pt, 0, sizeof(this->pt));
  this->count = 0;
}

void *
Hash__lookup (Hash *this, size_t key)
{
  void **pp = Hash__hash (this, key);
  return *pp;
}

void
Hash__add (Hash *this, size_t key, void *data)
{
  void **pp = Hash__hash (this, key);
  *pp = data;
  this->count ++;
}

void
Hash__remove (Hash *this, size_t key)
{
  void **pp = Hash__hash (this, key);
  *pp = NULL;
  this->count --;
}

void
Hash__getvals_1 (void **pt, int *count, int level)
{
  int i;
  for (i=0; i<256; i++)
    if (pt[i])
      {
	if (level == sizeof(size_t)-1)
	  (*count) ++;
	else
	  Hash__getvals_1 ((void **) pt[i], count, level+1);
      }
}

void
Hash__getvals_2 (void **pt, void ***table, int level)
{
  int i;
  for (i=0; i<256; i++)
    if (pt[i])
      {
	if (level == sizeof(size_t)-1)
	  *(*table)++ = pt[i];
	else
	  Hash__getvals_2 ((void **) pt[i], table, level+1);
      }
}

int
Hash__getvals (Hash *this, void ***vals)
{
  int count = 0;
  void **valtmp;
  Hash__getvals_1 (this->pt, &count, 0);
  printf("count is %d vs %d\n", count, this->count);
  *vals = (void **) malloc (count * sizeof (void *));
  valtmp = (*vals);
  Hash__getvals_2 (this->pt, &valtmp, 0);
  printf("vals is %ld\n", valtmp - *vals);
  return count;
}

//------------------------------------------------------------

typedef struct PerThread {
  int started;
  Buffer workload;
} PerThread;

void
PerThread__ctor (PerThread *this)
{
  this->started = 0;
  Buffer__ctor (&(this->workload));
}

void
PerThread__add (PerThread *this, byte x)
{
  Buffer__add (&(this->workload), x);
}

void
PerThread__add_int (PerThread *this, size_t x)
{
  Buffer__add_int (&(this->workload), x);
}

Hash *per_thread;

typedef enum {
  R_no_reason,
  R_not_seen,
  R_alloc,
  R_previously_freed,
  R_previously_realloced,
  R_realloc,
  R_memalign,
  R_posix_memalign
} Reasons;

const char *reasons_str[] = {
  "",
  "not seen",
  "alloc",
  "previously free'd",
  "previously realloc'd",
  "realloc",
  "memalign",
  "posix memalign"
};

typedef struct PerAddr {
  PerThread *owner;
  void *ptr;
  size_t idx;
  size_t reason_idx;
  struct __malloc_trace_buffer_s *inverted;
  unsigned char valid;
  unsigned char reason;
} PerAddr;

void
PerAddr__ctor (PerAddr *this, void *_ptr)
{
  this->owner = NULL;
  this->ptr = _ptr;
  this->valid = 0;
  this->reason = R_not_seen;
  this->inverted = NULL;
}

// Don't start at zero, zero is special.
size_t addr_count = 1;

Hash *per_addr;

PerAddr *
get_addr (void *ptr)
{
  PerAddr *p;
  if (ptr == NULL)
    return NULL;
  p = Hash__lookup (per_addr, (size_t)ptr);
  if (!p)
    {
      p = (PerAddr *) malloc (sizeof (PerAddr));
      PerAddr__ctor (p, ptr);
      Hash__add (per_addr, (size_t)ptr, p);
      p->idx = addr_count ++;
    }
  return p;
}

size_t sync_counter = 0;

// Insert a release/acquire pair to transfer ownership of data
// from thread TREL to thread TACK
void
sync_threads (PerThread *trel, PerThread *tacq)
{
  if (trel == tacq)
    return;
  sync_counter ++;
  PerThread__add (trel, C_SYNC_W);
  PerThread__add_int (trel, sync_counter);
  PerThread__add (tacq, C_SYNC_R);
  PerThread__add_int (tacq, sync_counter);
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

/* These are the state variables for the whole trace.  */
PerThread *master_thread = NULL;
int last_tid = -1;
PerThread *thread = NULL;
int pending_inversions = 0;
int fixed_inversions = 0;

static void
process_one_trace_record (struct __malloc_trace_buffer_s *r)
{
  size_t i = r - trace_records;

  // Quick-skip for NULs at EOF
  if (r->type == __MTB_TYPE_UNUSED)
    return;

  if (verbose > 1)
    printf("\033[32m%8x %2x (0x%p, 0x%x) =  0x%p\033[0m\n",
	   r->thread, r->type, r->ptr1, (int)r->size, r->ptr2);

  if (r->thread != last_tid)
    {
      thread = Hash__lookup (per_thread, r->thread);
      if (thread == NULL)
	{
	  thread = (PerThread *) malloc (sizeof (PerThread));
	  PerThread__ctor (thread);
	  Hash__add (per_thread, r->thread, thread);
	}
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
    case __MTB_TYPE_VALLOC:
    case __MTB_TYPE_PVALLOC:
      if (pa2 && pa2->valid)
	{
	  if (pa2->inverted)
	    {
	      printf ("%ld: pointer %p alloc'd again? (possible multi-level inversion) size %ld  %ld:%s\n",
		      i, pa2->ptr, (long int)r->size, pa2->reason_idx, reasons_str[pa2->reason]);
	      //	      exit (1);
	    }
	  pa2->inverted = r;
	  fixed_inversions ++;
	  pending_inversions ++;
	  return;
	}

      acq_ptr (thread, pa2);

      if (r->type == __MTB_TYPE_MALLOC)
	PerThread__add (thread, C_MALLOC);
      if (r->type == __MTB_TYPE_CALLOC)
	PerThread__add (thread, C_CALLOC);
      if (r->type == __MTB_TYPE_VALLOC)
	PerThread__add (thread, C_VALLOC);
      if (r->type == __MTB_TYPE_PVALLOC)
	PerThread__add (thread, C_PVALLOC);

      PerThread__add_int (thread, pa2 ? pa2->idx : 0);
      PerThread__add_int (thread, r->size);
      if (pa2)
	{
	  pa2->valid = 1;
	  pa2->reason = R_alloc;
	  pa2->reason_idx = i;
	}
      break;

    case __MTB_TYPE_FREE:
      acq_ptr (thread, pa1);
      if (pa1 == NULL)
	{
	  PerThread__add (thread, C_FREE);
	  PerThread__add_int (thread, 0);
	}
      else if (pa1->valid)
	{
	  PerThread__add (thread, C_FREE);
	  PerThread__add_int (thread, pa1->idx);
	  pa1->valid = 0;
	  pa1->reason = R_previously_freed;
	  pa1->reason_idx = i;

	  if (pa1->inverted)
	    {
	      process_one_trace_record (pa1->inverted);
	      pa1->inverted = NULL;
	      pending_inversions --;
	    }
	}
      else
	{
	  printf("%ld: invalid pointer %p passed to free: %ld:%s\n",
		 i, pa1->ptr, pa1->reason_idx, reasons_str[pa1->reason]);
	}
      break;

    case __MTB_TYPE_REALLOC:
      if (pa1 && pa1->owner)
	acq_ptr (thread, pa1);
      if (pa2 && pa2->owner)
	acq_ptr (thread, pa2);
      PerThread__add (thread, C_REALLOC);
      PerThread__add_int (thread, pa2 ? pa2->idx : 0);
      PerThread__add_int (thread, pa1 ? pa1->idx : 0);
      PerThread__add_int (thread, r->size);

      // handle inversion here too, eventually - both the alloc and free sides.
      if (pa1)
	{
	  pa1->valid = 0;
	  pa1->reason = R_previously_realloced;
	  pa1->reason_idx = i;
	}
      if (pa2)
	{
	  pa2->valid = 1;
	  pa2->reason = R_realloc;
	  pa2->reason_idx = i;
	}

      break;

    case __MTB_TYPE_MEMALIGN:
      acq_ptr (thread, pa2);
      if (pa2 && pa2->valid)
	printf ("%ld: pointer %p memalign'd again?  %ld:%s\n",
		i, pa2->ptr, pa2->reason_idx, reasons_str[pa2->reason]);
      PerThread__add (thread, C_MEMALIGN);
      PerThread__add_int (thread, pa2 ? pa2->idx : 0);
      PerThread__add_int (thread, r->size2);
      PerThread__add_int (thread, r->size);
      if (pa2)
	{
	  pa2->valid = 1;
	  pa2->reason = R_memalign;
	  pa2->reason_idx = i;
	}
      break;

    case __MTB_TYPE_POSIX_MEMALIGN:
      /* ptr1 is return value (0 or EINVAL etc)
	 ptr2 is address of allocated memory
	 size is the allocation size
	 size2 is the alignment */
      PerThread__add (thread, C_MEMALIGN);
      PerThread__add_int (thread, (r->ptr1 == 0) ? pa2->idx : 0);
      PerThread__add_int (thread, r->size2);
      PerThread__add_int (thread, r->size);
      if (r->ptr1 == 0)
	{
	  pa2->valid = 1;
	  pa2->reason = R_posix_memalign;
	  pa2->reason_idx = i;
	}
      break;

    }
}

//------------------------------------------------------------

static const char * const month_abbrevs[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static struct option longopts[] = {
  { "verbose", 0, NULL, 'v' },
  { "progress", 0, NULL, 'p' },
  { "file-buffers", 0, NULL, 'f' },
  { "tmpdir", 1, NULL, 't' },
  { NULL, 0, NULL, 0 }
};

static void
print_help (void)
{
  fprintf (stderr, "Usage: trace2wl [options] <outputfile.wl> <inputfile.mtrace>\n");
  fprintf (stderr, "  -v  --verbose        print stats about workload\n");
  fprintf (stderr, "  -p  --progress       show progress info\n");
  fprintf (stderr, "  -f  --file-buffers   use temporary files to store intermediate buffers\n");
  fprintf (stderr, "  -t <file>  --tmpdir=<file>   overrides $TMPDIR or /tmp\n");
  exit(1);
}

int
main(int argc, char **argv)
{
  int trace_fd, wl_fd;
  struct stat stbuf;
  unsigned long i;
  int opt;
  const char *tmp = NULL;
  size_t timer_divisor, old_percent_done=0;
  time_t start_time, cur_time;
  int show_progress = 0;

  while ((opt = getopt_long (argc, argv, "vhpft:", longopts, NULL)) != -1)
    {
      switch (opt) {
      case 'v':
	verbose ++;
	break;
      case 'p':
	show_progress ++;
	break;
      case 'f':
	use_file_buffers ++;
	break;
      case 't':
	tmp = optarg;
	printf("tmpdir: %s\n", tmp);
	break;
      case 'h':
      default:
	print_help();
      }
    }

  if (use_file_buffers)
    {
      if (tmp == NULL)
	{
	  tmp = getenv("TMPDIR");
	  printf("$TMPDIR: %s", tmp);
	  if (tmp == NULL)
	    {
	      tmp = "/tmp";
	      printf(" (using %s)", tmp);
	    }
	  printf("\n");
	}
      tmpdir = (char *) malloc (strlen(tmp) + strlen("/wl__") + 50);
      sprintf(tmpdir, "%s/wl_%d_", tmp, getpid());
      tmpdir_len = strlen(tmpdir);
    }

  per_addr = (Hash *) malloc (sizeof (Hash));
  Hash__ctor (per_addr);

  per_thread = (Hash *) malloc (sizeof (Hash));
  Hash__ctor (per_thread);

  if (argc-optind != 2)
    print_help();

  if (access (argv[optind], F_OK) == 0)
    {
      fprintf (stderr, "Error: output file %s already exists, will not overwrite\n", argv[optind]);
      exit(1);
    }

  trace_fd = open (argv[optind+1], O_RDONLY, 0666);
  if (trace_fd < 0)
    {
      fprintf (stderr, "Can't open %s for reading\n", argv[optind+1]);
      perror("The error was");
      exit(1);
    }

  if (stat (argv[optind+1], &stbuf) < 0)
    {
      fprintf (stderr, "Can't stat %s for reading\n", argv[optind+1]);
      perror("The error was");
      exit(1);
    }

  trace_records =
    (struct __malloc_trace_buffer_s *)
    mmap (NULL, stbuf.st_size, PROT_READ, MAP_SHARED, trace_fd, 0);
  if (trace_records == (void *)(-1))
    {
      fprintf (stderr, "Can't map %s for reading\n", argv[optind+1]);
      perror("The error was");
      exit(1);
    }
  num_trace_records = stbuf.st_size / sizeof(*trace_records);
  timer_divisor = num_trace_records / 100;
  time(&start_time);

  for (i = 0; i < num_trace_records; i++)
    {
      if (show_progress)
	{
	  int percent_done = i / timer_divisor;
	  if (percent_done != old_percent_done)
	    {
	      struct tm *tm;
	      old_percent_done = percent_done;
	      time(&cur_time);
	      cur_time = cur_time + (cur_time - start_time) * 100 / percent_done;
	      tm = localtime(&cur_time);
	      printf(" %3d%% done, ETA %3s %2d %2d:%02d \r", percent_done,
		     month_abbrevs[tm->tm_mon], tm->tm_mday,
		     tm->tm_hour, tm->tm_min);
	      fflush (stdout);
	    }
	}
      process_one_trace_record (trace_records + i);
    }
  if (show_progress)
    printf(" 100%% done, writing out buffers...\033[K\n");

  PerThread **threads;
  int n_threads;
  n_threads = Hash__getvals (per_thread, (void ***)&threads);

  size_t *thread_off = (size_t *) malloc (n_threads * sizeof (size_t));

  if(verbose)
    printf("%d threads\n", n_threads);
  for (i = 0; i < n_threads; i ++)
    {
      PerThread__add(threads[i], C_DONE);
      if(verbose)
	printf("thread: %ld bytes\n", (long)threads[i]->workload.count_total);
    }

  /* The location of each thread's workload depends on the size of the
     startup block, but the size of the startup block depends on the
     size of the thread's location encoding.  So, we loop until it
     stabilizes.  */
  size_t old_len = 1;
  size_t new_len = 2;
  Buffer main_loop;
  Buffer__ctor (&main_loop);
  while (old_len != new_len)
    {
      size_t off = new_len;
      int i;

      old_len = new_len;
      Buffer__clear (&main_loop);

      Buffer__add (&main_loop, C_ALLOC_PTRS);
      Buffer__add_int (&main_loop, addr_count);
      Buffer__add (&main_loop, C_ALLOC_SYNCS);
      Buffer__add_int (&main_loop, sync_counter);
      Buffer__add (&main_loop, C_NTHREADS);
      Buffer__add_int (&main_loop, n_threads);

      for (i=0; i<n_threads; i++)
	{
	  thread_off[i] = off;
	  Buffer__add (&main_loop, C_START_THREAD);
	  Buffer__add_int (&main_loop, off);
	  off += threads[i]->workload.count_total;
	}

      Buffer__add (&main_loop, C_DONE);

      new_len = main_loop.count_total;
    }

  wl_fd = open (argv[optind], O_CREAT|O_EXCL|O_RDWR, 0666);
  if (wl_fd < 0)
    {
      fprintf (stderr, "Can't open %s for writing\n", argv[optind]);
      perror("The error was");
      exit(1);
    }

  Buffer__write (&main_loop, wl_fd);

  for (i=0; i<n_threads; i++)
    {
      if (verbose || show_progress)
	printf("Start thread[%ld] offset 0x%lx\n", i, (long)thread_off[i]);
      Buffer__write (&(threads[i]->workload), wl_fd);
    }

  close (wl_fd);

  if (pending_inversions)
    printf("%d pending inversions remain\n", pending_inversions);
  if (fixed_inversions)
    printf("%d inversions fixed\n", fixed_inversions);

  return 0;
}
