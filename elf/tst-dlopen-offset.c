#include <dlfcn.h>
#include <stdio.h>

/* These numbers need to be coordinated with the offsets passed to make the combined .so.  */
int offa = 64;
int offb = 128;
int offc = 192;

int
do_test (void)
{
  void *p1 = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", offa * 1024, RTLD_LAZY);

  if (!p1)
    {
      puts (dlerror ());
      return 1;
    }

  int (*f) (void) = dlsym (p1, "foo");
  if (f)
    {
      (*f)();
    }
  else
    {
      puts (dlerror ());
      return 1;
    }

  void *p2 = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", offb * 1024, RTLD_LAZY);

  int (*bar) (void) = dlsym (p2, "bar");
  if (bar)
    {
      (*bar)();
    }
  else
    {
      puts (dlerror ());
      return 1;
    }

  void *p3 = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", offc * 1024, RTLD_LAZY);

  int (*xyzzy) (void) = dlsym (p3, "xyzzy");
  if (xyzzy)
    {
      (*xyzzy)();
    }
  else
    {
      puts (dlerror ());
      return 1;
    }

  if (p1)
    dlclose (p1);

  p1 = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", offa * 1024, RTLD_LAZY);

  f = dlsym (p1, "someothersym");
  if (!f)
    {
      puts (dlerror ());
      puts (" (expected)");
    }
  else
    {
      puts ("Symbol found unexpectedly");
      return 1;
    }

  f = dlsym (p1, "xyzzy");
  if (!f)
    {
      puts (dlerror ());
      puts (" (expected)");
    }
  else
    {
      puts ("Symbol found unexpectedly");
      return 1;
    }

  p1 = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", offa * 1024, RTLD_LAZY);

  f = dlsym (p1, "foo");
  if (f)
    {
      (*f)();
    }
  else
    {
      puts (dlerror ());
      return 1;
    }

  void *px = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-comb.so", 0, RTLD_LAZY);

  if (!px)
    {
      puts (dlerror ());
      puts (" (expected)");
    }
  else
    {
      puts ("dlopen_with_offset succeeded unexpectedly");
      return 1;
    }

  px = __google_dlopen_with_offset ("$ORIGIN/tst-dlopen-offset-mod1.so", 0, RTLD_LAZY);

  f = dlsym (px, "foo");
  if (f)
    {
      (*f)();
    }
  else
    {
      puts (dlerror ());
      return 1;
    }

  px = __google_dlopen_with_offset ("$ORIGIN/nonexistent.so", 0, RTLD_LAZY);

  if (!px)
    {
      puts (dlerror ());
      puts (" (expected)");
    }
  else
    {
      puts ("dlopen_with_offset succeeded unexpectedly");
      return 1;
    }

  return 0;
}

#define TIMEOUT 100

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
