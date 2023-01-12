#ifndef _CLONE_INTERNAL_H
#define _CLONE_INTERNAL_H

#include <clone3.h>

/* The clone3 syscall provides a superset of the functionality of the clone
   interface.  The kernel might extend __CL_ARGS struct in the future, with
   each version with a diffent __SIZE.  If the child is created, it will
   start __FUNC function with __ARG arguments.

   Different than kernel, the implementation also returns EINVAL for an
   invalid NULL __CL_ARGS or __FUNC (similar to __clone).

   This function is only implemented if the ABI defines HAVE_CLONE3_WRAPPER.
*/
extern int __clone3 (struct clone_args *__cl_args, size_t __size,
		     int (*__func) (void *__arg), void *__arg);

/* The internal wrapper of clone/clone2 and clone3.  Different than __clone3,
   it will align the stack if required.  If __clone3 returns -1 with ENOSYS,
   fall back to clone or clone2.  */
extern int __clone_internal (struct clone_args *__cl_args,
			     int (*__func) (void *__arg), void *__arg);

#ifndef _ISOMAC
libc_hidden_proto (__clone3)
libc_hidden_proto (__clone_internal)
#endif

#endif
