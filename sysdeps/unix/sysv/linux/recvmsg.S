#define	socket	recvmsg
#define	__socket __libc_recvmsg
#include <socket.S>
weak_alias (__libc_recvmsg, __recvmsg)
