#ifndef	_MACH_INIT_H

#define	_MACH_INIT_H	1

#include <mach/mach_types.h>

/* Return the current task's task port.  */
extern mach_port_t __mach_task_self (void);

/* This cache is initialized at startup.  */
extern mach_port_t __mach_task_self_;
#define mach_task_self()	__mach_task_self_

/* Kernel page size..  */
extern vm_size_t __vm_page_size;

/* Round the address X up to a page boundary.  */
#define round_page(x)	\
  ((((vm_offset_t) (x) + __vm_page_size - 1) / __vm_page_size) * \
   __vm_page_size)

/* Truncate the address X down to a page boundary.  */
#define trunc_page(x)	\
  ((((vm_offset_t) (x)) / __vm_page_size) * __vm_page_size)


#endif	/* mach_init.h */
