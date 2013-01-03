/* Test function pointer to local STT_GNU_IFUNC function.  */

extern void (*foo_ptr) (void);

int
main (void)
{
  foo_ptr ();
  return 0;
}
