/* We must remove the environment variables introduced for the a.out
   dynamic loading.  */
#define EXTRA_UNSECURE_ENVVARS \
  do {									      \
    if (__libc_enable_secure)						      \
      {									      \
	unsetenv ("LD_AOUT_LIBRARY_PATH");				      \
	unsetenv ("LD_AOUT_PRELOAD");					      \
      }									      \
  } while (0)
