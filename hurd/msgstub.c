#include <hurd.h>
#include <hurd/msg_server.h>

/* XXX */
#define STUB(fn) error_t fn (mach_port_t port) { return EOPNOTSUPP; }

STUB(_S_get_init_ports)
STUB(_S_set_init_ports)
STUB(_S_get_init_ints)
STUB(_S_set_init_ints)
STUB(_S_get_init_int)
STUB(_S_set_init_int)
STUB(_S_get_dtable)
STUB(_S_set_dtable)
STUB(_S_get_fd)
STUB(_S_set_fd)
STUB(_S_io_select_done)
STUB(_S_startup_dosync)

