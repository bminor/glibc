#define	socket	recvmsg
#include <socket.S>
strong_alias (__recvmsg, __libc_recvmsg)
