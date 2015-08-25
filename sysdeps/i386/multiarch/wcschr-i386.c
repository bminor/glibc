#include <wchar.h>

#if IS_IN (libc)
# undef libc_hidden_weak
# define libc_hidden_weak(name)

# undef weak_alias
# define weak_alias(name,alias)

# ifdef SHARED
#  undef libc_hidden_def
#  define libc_hidden_def(name) \
   __hidden_ver1 (__wcschr_i386, __GI_wcschr, __wcschr_i386); \
   strong_alias (__wcschr_i386, __wcschr_i386_1); \
   __hidden_ver1 (__wcschr_i386_1, __GI___wcschr, __wcschr_i386_1);
# endif
#endif

extern __typeof (wcschr) __wcschr_i386;

#define WCSCHR  __wcschr_i386
#include <wcsmbs/wcschr.c>
