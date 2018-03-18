#ifndef	_GLOB_H
#include <posix/glob.h>

#ifndef _ISOMAC
libc_hidden_proto (glob)
libc_hidden_proto (glob64)
libc_hidden_proto (globfree)
libc_hidden_proto (globfree64)

/* Now define the internal interfaces.  */
extern typeof (glob_pattern_p) __glob_pattern_p;
libc_hidden_proto (__glob_pattern_p)
extern typeof (glob) __glob;
libc_hidden_proto (__glob)
extern typeof (glob) __glob_lstat_compat;
libc_hidden_proto (__glob_lstat_compat)
extern typeof (glob64) __glob64;
libc_hidden_proto (__glob64)
extern typeof (glob64) __glob64_lstat_compat;
libc_hidden_proto (__glob64_lstat_compat)
#endif

#endif
