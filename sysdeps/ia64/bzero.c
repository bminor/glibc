/* ia64 does not export __bzero symbol.  */
#define __bzero bzero
#include <string/bzero.c>
