#define	socket	send
#define	__socket __libc_send
#include <socket.S>
weak_alias (__libc_send, __send)
