#ifndef _MALLOC_H

#include <malloc/malloc.h>

# ifndef _ISOMAC
#  include <rtld-malloc.h>

/* In the GNU libc we rename the global variable
   `__malloc_initialized' to `__libc_malloc_initialized'.  */
#define __malloc_initialized __libc_malloc_initialized
/* Nonzero if the malloc is already initialized.  */
extern int __malloc_initialized attribute_hidden;

struct malloc_state;
typedef struct malloc_state *mstate;

# endif /* !_ISOMAC */

#ifdef USE_MTAG
extern int __mtag_mmap_flags;
#define MTAG_MMAP_FLAGS __mtag_mmap_flags
#else
#define MTAG_MMAP_FLAGS 0
#endif

#endif
