#define	socket	connect
#define NARGS 3
#define __socket __libc_connect
#include <socket.S>
weak_alias (__libc_connect, __connect)
