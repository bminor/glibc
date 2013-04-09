#include <string.h>

__typeof (strncasecmp) __strncasecmp_power7
  __attribute__ ((__target__ ("cpu=power7")));

#define __strncasecmp __strncasecmp_power7
#include <string/strncase.c>
