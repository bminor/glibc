#define	socket	sendto
#define NARGS 6
#define	__socket __libc_sendto
#include <socket.S>
weak_alias (__libc_sendto, __sendto)
