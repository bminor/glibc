#define	socket	sendto
#include <socket.S>
strong_alias (__sendto, __libc_sendto)
