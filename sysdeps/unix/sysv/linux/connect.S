#define	socket	connect
#include <socket.S>
strong_alias (__connect, __libc_connect)
