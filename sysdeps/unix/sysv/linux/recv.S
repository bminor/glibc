#define	socket	recv
#define NARGS 4
#define	__socket __libc_recv
#include <socket.S>
weak_alias (__libc_recv, __recv)
