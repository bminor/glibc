#ifndef _FCNTL_H
#include <io/fcntl.h>

/* Now define the internal interfaces.  */
extern int __open64 __P ((__const char *__file, int __oflag, ...));
extern int __libc_open64 __P ((const char *file, int oflag, ...));
extern int __libc_open __P ((const char *file, int oflag, ...));
extern int __libc_fcntl __P ((int fd, int cmd, ...));
#endif
