#define	socket	sendmsg
#define	__socket __libc_sendmsg
#include <socket.S>
weak_alias (__libc_sendmsg, __sendmsg)
