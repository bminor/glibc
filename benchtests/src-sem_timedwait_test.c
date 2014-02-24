sem_t sem;

void
init_test (void)
{
  sem_init (&sem, 0, 0);
}

static inline void __always_inline
sem_timedwait_test (bool contended)
{
  struct timespec t;
  clock_gettime (CLOCK_REALTIME, &t);
  t.tv_nsec += 10000;
  if (!contended)
    sem_post (&sem);
  sem_timedwait (&sem, &t);
}
