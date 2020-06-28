#ifndef	_HURD_H
#include_next <hurd.h>

/* Like __USEPORT, but cleans fd on cancel.  */
#define	__USEPORT_CANCEL(which, expr) \
  HURD_PORT_USE_CANCEL (&_hurd_ports[INIT_PORT_##which], (expr))

#ifndef _ISOMAC
libc_hidden_proto (_hurd_exec_paths)
libc_hidden_proto (_hurd_init)
#endif
#endif
