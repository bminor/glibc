#define	socket	sendmsg
#define NARGS 3
#define	__socket __libc_sendmsg
#include <socket.S>
weak_alias (__libc_sendmsg, __sendmsg)
