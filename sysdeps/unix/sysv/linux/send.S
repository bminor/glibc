#define	socket	send
#include <socket.S>
strong_alias (__send, __libc_send)
