#define	socket	recv
#include <socket.S>
strong_alias (__recv, __libc_recv)
