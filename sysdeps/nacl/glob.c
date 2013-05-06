/* XXX temporary? hack */
#include <unistd.h>
#define getlogin_r(name, len) (ENOSYS)

/* Fetch the version that defines glob64 as an alias.  */
#include <sysdeps/wordsize-64/glob.c>
