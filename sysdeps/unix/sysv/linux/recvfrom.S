#define	socket	recvfrom
#include <socket.S>
strong_alias (__recvfrom, __libc_recvfrom)
