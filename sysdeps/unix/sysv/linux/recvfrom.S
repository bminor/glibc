#define	socket	recvfrom
#define NARGS 6
#define	__socket __libc_recvfrom
#include <socket.S>
weak_alias (__libc_recvfrom, __recvfrom)
