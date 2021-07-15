#include <dlfcn.h>
#include <stdio.h>

int
main (void)
{
  void *h = dlopen ("$ORIGIN/unload8mod1.so", RTLD_LAZY);
  if (h == NULL)
    {
      puts ("dlopen unload8mod1.so failed");
      return 1;
    }

  void *h2 = dlopen ("$ORIGIN/unload8mod1x.so", RTLD_LAZY);
  if (h2 == NULL)
    {
      puts ("dlopen unload8mod1x.so failed");
      return 1;
    }
  dlclose (h2);

  int (*mod1) (void) = dlsym (h, "mod1");
  if (mod1 == NULL)
    {
      puts ("dlsym failed");
      return 1;
    }

  mod1 ();

  // Additional test to detect when the fastload hash table has bad pointers to
  // names of unloaded libraries hanging around in it.

  int (*mod1b) (void) = dlsym (h, "mod1b");
  if (mod1b == NULL)
    {
      puts ("dlsym failed");
      return 1;
    }

  mod1b ();

  dlclose (h);

  void *h2x = dlopen ("$ORIGIN/unload8mod2.so", RTLD_LAZY);
  if (h2x == NULL)
    {
      puts ("dlopen unload8mod2.so failed");
      return 1;
    }


  void *h2xx = dlopen ("$ORIGIN/unload8mod1x.so", RTLD_LAZY);
  if (h2xx == NULL)
    {
      puts ("dlopen unload8mod1x.so failed");
      return 1;
    }

  dlclose (h);
  dlclose (h2x);
  dlclose (h2xx);

  void *h3xx = dlopen ("$ORIGIN/unload8mod3.so", RTLD_LAZY);
  if (h3xx == NULL)
    {
      puts ("dlopen unload8mod3.so failed");
      return 1;
    }

  dlclose (h3xx);

  return 0;
}
