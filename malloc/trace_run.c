#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>

#include "malloc.h"

/* These must stay in sync with trace2dat */
#define C_NOP 0
#define C_DONE 1
#define C_MALLOC 2
#define C_CALLOC 3
#define C_REALLOC 4
#define C_FREE 5
#define C_SYNC_W 6
#define C_SYNC_R 7
#define C_ALLOC_PTRS 8
#define C_ALLOC_SYNCS 9
#define C_NTHREADS 10
#define C_START_THREAD 11

static __inline__ int64_t rdtsc_s(void)
{
  unsigned a, d;
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  asm volatile("rdtsc" : "=a" (a), "=d" (d));
  return ((unsigned long)a) | (((unsigned long)d) << 32);
}

static __inline__ int64_t rdtsc_e(void)
{
  unsigned a, d;
  asm volatile("rdtscp" : "=a" (a), "=d" (d));
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  return ((unsigned long)a) | (((unsigned long)d) << 32);
}

static int64_t diff_timeval (struct timeval e, struct timeval s)
{
  int64_t usec;
  if (e.tv_usec < s.tv_usec)
    usec = (e.tv_usec + 1000000 - s.tv_usec) + (e.tv_sec-1 - s.tv_sec)*1000000;
  else
    usec = (e.tv_usec - s.tv_usec) + (e.tv_sec - s.tv_sec)*1000000;
  return usec;
}

#if 1
#define Q1
#define Q2
#else
pthread_mutex_t genmutex = PTHREAD_MUTEX_INITIALIZER;
#define Q1   pthread_mutex_lock(&genmutex)
#define Q2   pthread_mutex_unlock(&genmutex)
#endif

pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
#define NCBUF 10
static char cbuf[NCBUF][30];
static int ci = 0;

char *comma(int64_t x)
{
  char buf[30], *bs, *bd;
  int l, i, idx;

  pthread_mutex_lock(&cmutex);
  ci = (ci + 1) % NCBUF;
  idx = ci;
  pthread_mutex_unlock(&cmutex);
  bs = buf;
  bd = cbuf[idx];

  sprintf(buf, "%lld", x);
  l = strlen(buf);
  i = l;
  while (*bs)
    {
      *bd++ = *bs++;
      i--;
      if (i % 3 == 0 && *bs)
	*bd++ = ',';
    }
  *bd = 0;
  return cbuf[idx];
}

static volatile void **ptrs;
static size_t n_ptrs;
static volatile char *syncs;
static pthread_mutex_t *mutexes;
static pthread_cond_t *conds;
static size_t n_syncs;
static unsigned char *data;
static size_t n_data;

static pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;
int64_t malloc_time = 0, malloc_count = 0;
int64_t calloc_time = 0, calloc_count = 0;
int64_t realloc_time = 0, realloc_count = 0;
int64_t free_time = 0, free_count = 0;

pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
int threads_done = 0;

//#define dprintf printf
#define dprintf(...) 1

//#define mprintf printf
#define mprintf(...) 1

#define myabort() my_abort_2(me, __LINE__)
my_abort_2 (pthread_t me, int line)
{
  fprintf(stderr, "Abort thread %d at line %d\n", (int)me, line);
  abort();
}

static void
wmem (volatile void *ptr, int count)
{
  char *p = (char *)ptr;
  int i;
  size_t sz;

  if (!p)
    return;

  //  sz = *((size_t *)ptr-1) & ~7;
  //  fprintf(stderr, "wmem: %p size %x csize %x\n", ptr,
  //	  count, sz);
  //  if (sz < 4*sizeof(size_t))
  //    abort();
  for (i=0; i<count; i+=8)
    p[i] = 0x11;
}
#define xwmem(a,b)

static size_t get_int (unsigned char **ptr)
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

static void *
thread_common (void *my_data_v)
{
  pthread_t me = pthread_self ();
  size_t p1, p2, sz;
  unsigned char *cp = my_data_v;
  int64_t my_malloc_time = 0, my_malloc_count = 0;
  int64_t my_calloc_time = 0, my_calloc_count = 0;
  int64_t my_realloc_time = 0, my_realloc_count = 0;
  int64_t my_free_time = 0, my_free_count = 0;
  int64_t stime;
  volatile void *tmp;

  while (1)
    {
      if (cp > data + n_data)
	myabort();
      dprintf("op %d:%d is %d\n", (int)me, cp-data, *cp);
      switch (*cp++)
	{
	case C_NOP:
	  break;

	case C_DONE:
	  dprintf("op %d:%d DONE\n", (int)me, cp-data);
	  pthread_mutex_lock (&stat_mutex);
	  malloc_time += my_malloc_time;
	  calloc_time += my_calloc_time;
	  realloc_time += my_realloc_time;
	  free_time += my_free_time;
	  malloc_count += my_malloc_count;
	  calloc_count += my_calloc_count;
	  realloc_count += my_realloc_count;
	  free_count += my_free_count;
	  threads_done ++;
	  pthread_mutex_unlock (&stat_mutex);
	  pthread_mutex_lock(&stop_mutex);
	  pthread_mutex_unlock(&stop_mutex);
	  return NULL;

	case C_MALLOC:
	  p2 = get_int (&cp);
	  sz = get_int (&cp);
	  dprintf("op %d:%d %d = MALLOC %d\n", (int)me, cp-data, p2, sz);
	  if (p2 > n_ptrs)
	    myabort();
	  stime = rdtsc_s();
	  Q1;
	  if (ptrs[p2])
	    free ((void *)ptrs[p2]);
	  ptrs[p2] = malloc (sz);
	  mprintf("%p = malloc(%lx)\n", ptrs[p2], sz);
	  Q2;
	  my_malloc_time += rdtsc_e() - stime;
	  my_malloc_count ++;
	  wmem(ptrs[p2], sz);
	  break;

	case C_CALLOC:
	  p2 = get_int (&cp);
	  sz = get_int (&cp);
	  dprintf("op %d:%d %d = CALLOC %d\n", (int)me, cp-data, p2, sz);
	  if (p2 > n_ptrs)
	    myabort();
	  stime = rdtsc_s();
	  Q1;
	  if (ptrs[p2])
	    free ((void *)ptrs[p2]);
	  ptrs[p2] = calloc (sz, 1);
	  mprintf("%p = calloc(%lx)\n", ptrs[p2], sz);
	  Q2;
	  my_calloc_time += rdtsc_e() - stime;
	  my_calloc_count ++;
	  wmem(ptrs[p2], sz);
	  break;

	case C_REALLOC:
	  p2 = get_int (&cp);
	  p1 = get_int (&cp);
	  sz = get_int (&cp);
	  dprintf("op %d:%d %d = REALLOC %d %d\n", (int)me, cp-data, p2, p1, sz);
	  if (p1 > n_ptrs)
	    myabort();
	  if (p2 > n_ptrs)
	    myabort();
	  stime = rdtsc_s();
	  Q1;
	  tmp = ptrs[p1];
	  ptrs[p2] = realloc ((void *)ptrs[p1], sz);
	  mprintf("%p = relloc(%p,%lx)\n", ptrs[p2], tmp,sz);
	  Q2;
	  my_realloc_time += rdtsc_e() - stime;
	  my_realloc_count ++;
	  wmem(ptrs[p2], sz);
	  if (p1 != p2)
	    ptrs[p1] = 0;
	  break;

	case C_FREE:
	  p1 = get_int (&cp);
	  if (p1 > n_ptrs)
	    myabort();
	  dprintf("op %d:%d FREE %d\n", (int)me, cp-data, p1);
	  stime = rdtsc_s();
	  Q1;
	  mprintf("free(%p)\n", ptrs[p1]);
	  free ((void *)ptrs[p1]);
	  Q2;
	  my_free_time += rdtsc_e() - stime;
	  my_free_count ++;
	  ptrs[p1] = 0;
	  break;

	case C_SYNC_W:
	  p1 = get_int(&cp);
	  dprintf("op %d:%d SYNC_W %d\n", (int)me, cp-data, p1);
	  if (p1 > n_syncs)
	    myabort();
	  pthread_mutex_lock (&mutexes[p1]);
	  syncs[p1] = 1;
	  pthread_cond_signal (&conds[p1]);
	  __sync_synchronize ();
	  pthread_mutex_unlock (&mutexes[p1]);
	  break;

	case C_SYNC_R:
	  p1 = get_int(&cp);
	  dprintf("op %d:%d SYNC_R %d\n", (int)me, cp-data, p1);
	  if (p1 > n_syncs)
	    myabort();
	  pthread_mutex_lock (&mutexes[p1]);
	  while (syncs[p1] != 1)
	    {
	      pthread_cond_wait (&conds[p1], &mutexes[p1]);
	      __sync_synchronize ();
	    }
	  pthread_mutex_unlock (&mutexes[p1]);
	  break;

	default:
	  myabort();
	}
    }
}

static void *alloc_mem (size_t amt)
{
  void *rv = mmap (NULL, amt, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  mlock (rv, amt);
  memset (rv, 0, amt);
  return rv;
}

static pthread_t *thread_ids;

void *
my_malloc (char *msg, int size, unsigned char **cp, size_t *psz, size_t count)
{
  void *rv;
  if (psz)
    count = *psz = get_int (cp);
  dprintf ("my_malloc for %s size %d * %d\n", msg, size, count);
  rv = alloc_mem(size * count);
  if (!rv)
    {
      fprintf(stderr, "calloc(%lu,%lu) failed\n", (long unsigned)size, (long unsigned)*psz);
      exit(1);
    }
  mlock (rv, size * count);
  return rv;
}

static const char * const scan_names[] = {
  "UNUSED",
  "ARENA",
  "HEAP",
  "CHUNK_USED",
  "CHUNK_FREE",
  "FASTBIN_FREE",
  "UNSORTED",
  "TOP",
  "TCACHE",
  "USED"
};

void
malloc_scan_callback (void *ptr, size_t length, int type)
{
  printf("%s: ptr %p length %llx\n", scan_names[type], ptr, length);
}

#define MY_ALLOC(T, psz)				\
  (typeof (T)) my_malloc (#T, sizeof(*T), &cp, psz, 0)
#define MY_ALLOCN(T, count)				\
  (typeof (T)) my_malloc (#T, sizeof(*T), &cp, NULL, count)

int
main(int argc, char **argv)
{
  int64_t start;
  int64_t end;
  int64_t usec;
  struct timeval tv_s, tv_e;
  int fd;
  struct stat statb;
  unsigned char *cp;
  int thread_idx = 0;
  int i;
  size_t n_threads;
  size_t idx;
  struct rusage res_start, res_end;

  if (argc < 2)
    {
      fprintf(stderr, "Usage: %s <trace2dat.outfile>\n", argv[0]);
      exit(1);
    }
  fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    {
      fprintf(stderr, "Unable to open %s for reading\n", argv[1]);
      perror("The error was");
      exit(1);
    }
  fstat (fd, &statb);

  n_data = statb.st_size;
  data = mmap (NULL, statb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  mlock (data, statb.st_size);
  for (i=0; i<n_data; i+=512)
    asm volatile ("# forced read %0" :: "r" (data[i]));

  pthread_mutex_lock(&stop_mutex);

  cp = data;
  while (cp)
    {
      switch (*cp++)
	{
	case C_NOP:
	  break;
	case C_ALLOC_PTRS:
	  ptrs = MY_ALLOC (ptrs, &n_ptrs);
	  ptrs[0] = 0;
	  break;
	case C_ALLOC_SYNCS:
	  n_syncs = get_int(&cp);
	  syncs = MY_ALLOCN (syncs, n_syncs);
	  conds = MY_ALLOCN (conds, n_syncs);
	  mutexes = MY_ALLOCN (mutexes, n_syncs);
	  for (idx=0; idx<n_syncs; idx++)
	    {
	      pthread_mutex_init (&mutexes[idx], NULL);
	      pthread_cond_init (&conds[idx], NULL);
	    }
	  break;
	case C_NTHREADS:
	  thread_ids = MY_ALLOC (thread_ids, &n_threads);
	  /* The next thing in the workscript is thread creation */
	  getrusage (RUSAGE_SELF, &res_start);
	  gettimeofday (&tv_s, NULL);
	  start = rdtsc_s();
	  break;
	case C_START_THREAD:
	  idx = get_int (&cp);
	  pthread_create (&thread_ids[thread_idx], NULL, thread_common, data + idx);
	  dprintf("Starting thread %lld at offset %d %x\n", (long)thread_ids[thread_idx], (int)idx, (unsigned int)idx);
	  thread_idx ++;
	  break;
	case C_DONE:
	  do
	    {
	      pthread_mutex_lock (&stat_mutex);
	      i = threads_done;
	      pthread_mutex_unlock (&stat_mutex);
	    } while (i < thread_idx);
	  cp = NULL;
	  break;
	}
    }
  end = rdtsc_e();
  gettimeofday (&tv_e, NULL);
  getrusage (RUSAGE_SELF, &res_end);

  printf("%s cycles\n", comma(end - start));
  usec = diff_timeval (tv_e, tv_s);
  printf("%s usec wall time\n", comma(usec));

  usec = diff_timeval (res_end.ru_utime, res_start.ru_utime);
  printf("%s usec across %d thread%s\n", comma(usec), n_threads, n_threads == 1 ? "" : "s");
  printf("%s Kb Max RSS (%s -> %s)\n",
	 comma(res_end.ru_maxrss - res_start.ru_maxrss),
	 comma(res_start.ru_maxrss), comma(res_end.ru_maxrss));

  if (malloc_count == 0) malloc_count ++;
  if (calloc_count == 0) calloc_count ++;
  if (realloc_count == 0) realloc_count ++;
  if (free_count == 0) free_count ++;

  printf("\n");
  printf("Avg malloc time: %6s in %10s calls\n", comma(malloc_time/malloc_count), comma(malloc_count));
  printf("Avg calloc time: %6s in %10s calls\n", comma(calloc_time/calloc_count), comma(calloc_count));
  printf("Avg realloc time: %5s in %10s calls\n", comma(realloc_time/realloc_count), comma(realloc_count));
  printf("Avg free time: %8s in %10s calls\n", comma(free_time/free_count), comma(free_count));
  printf("Total call time: %s cycles\n", comma(malloc_time+calloc_time+realloc_time+free_time));
  printf("\n");

#if 0
  /* Free any still-held chunks of memory.  */
  for (idx=0; idx<n_ptrs; idx++)
    if (ptrs[idx])
      {
	free((void *)ptrs[idx]);
	ptrs[idx] = 0;
      }
#endif

  /* This will fail (crash) for system glibc but that's OK.  */
  __malloc_scan_chunks(malloc_scan_callback);

  malloc_info (0, stdout);

#if 1
  /* ...or report them as used.  */
  for (idx=0; idx<n_ptrs; idx++)
    if (ptrs[idx])
      {
	char *p = (char *)ptrs[idx] - 2*sizeof(size_t);
	size_t *sp = (size_t *)p;
	size_t size = sp[1] & ~7;
	malloc_scan_callback (sp, size, 9);
      }
#endif

  /* Now that we've scanned all the per-thread caches, it's safe to
     let them exit and clean up.  */
  pthread_mutex_unlock(&stop_mutex);

  for (i=0; i<thread_idx; i++)
    pthread_join (thread_ids[i], NULL);

  return 0;
}
