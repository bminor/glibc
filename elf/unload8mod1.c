#include <dlfcn.h>

extern void mod2 (void);

void
mod1 (void)
{
  mod2 ();
}

int
mod1b (void)
{
  void *h = dlopen ("$ORIGIN/unload8mod3.so", RTLD_LAZY);
  if (h == NULL)
    {
      puts ("dlopen unload8mod3.so failed");
      return 1;
    }
  return 0;
}
