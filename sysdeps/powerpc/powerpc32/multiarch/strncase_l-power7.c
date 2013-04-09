#include <string.h>

__typeof (strncasecmp_l) __strncasecmp_l_power7
  __attribute__ ((__target__ ("cpu=power7")));

#define __strncasecmp_l __strncasecmp_l_power7
#define USE_IN_EXTENDED_LOCALE_MODEL    1
#include "string/strncase.c"
