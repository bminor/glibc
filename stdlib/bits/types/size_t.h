#ifndef __size_t_defined
#define __size_t_defined 1

/* We rely on the compiler's stddef.h to define size_t for us.  */
#define __need_size_t
#include <stddef.h>
#undef __need_size_t

#endif
