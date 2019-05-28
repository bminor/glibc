#ifndef __ptrdiff_t_defined
#define __ptrdiff_t_defined 1

/* We rely on the compiler's stddef.h to define ptrdiff_t for us.  */
#define __need_ptrdiff_t
#include <stddef.h>
#undef __need_ptrdiff_t

#endif
