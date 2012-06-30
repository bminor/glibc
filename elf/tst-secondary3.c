/* Verify that weak function is called.  */

#include "tst-secondary.h"

extern int bar (void);
extern void foo (void);

int
main (void)
{
  foo ();
  return bar () != WEAK;
}
