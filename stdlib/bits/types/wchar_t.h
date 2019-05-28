#ifndef __wchar_t_defined
#define __wchar_t_defined 1

/* We rely on the compiler's stddef.h to define wchar_t for us.  */
#define __need_wchar_t
#include <stddef.h>
#undef __need_wchar_t

#endif
