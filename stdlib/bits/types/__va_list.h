#ifndef ____va_list_defined
#define ____va_list_defined 1

/* We rely on the compiler's stdarg.h to define __gnuc_va_list for us.  */
#define __need___va_list
#include <stdarg.h>
#undef __need___va_list

#endif
