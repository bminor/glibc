#define	socket	accept
#define	__socket __libc_accept
#include <socket.S>
weak_alias (__libc_accept, __accept)
