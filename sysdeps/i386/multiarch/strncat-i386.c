#define STRNCAT __strncat_i386
#ifdef SHARED
#undef libc_hidden_def
#define libc_hidden_def(name) \
  __hidden_ver1 (__strncat_i386, __GI___strncat, __strncat_i386);
#endif

#include "string/strncat.c"
