#include <mach_init.h>

extern void __mig_init (int);

mach_port_t __mach_task_self_;
vm_size_t __vm_page_size;

void
__mach_init (void)
{
  error_t err;
  vm_statistics_data_t stats;

  __mach_task_self_ = (__mach_task_self) ();
  __mig_init (0);

  if (err = __vm_statistics (__mach_task_self (), &stats))
    _exit (err);
  __vm_page_size = stats.pagesize;
}
