#define	socket	accept
#include <socket.S>
strong_alias (__accept, __libc_accept)
