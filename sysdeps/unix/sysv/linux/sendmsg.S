#define	socket	sendmsg
#include <socket.S>
strong_alias (__sendmsg, __libc_sendmsg)
