#ifndef _ERR_H
#include <misc/err.h>

/* Prototypes for internal err.h functions.  */
void
__vwarnx_internal (const char *format, __gnuc_va_list ap,
		   unsigned int mode_flags);

void
__vwarn_internal (const char *format, __gnuc_va_list ap,
		   unsigned int mode_flags);

# ifndef _ISOMAC

libc_hidden_proto (warn)
libc_hidden_proto (warnx)
libc_hidden_proto (vwarn)
libc_hidden_proto (vwarnx)
libc_hidden_proto (verr)
libc_hidden_proto (verrx)

# endif /* !_ISOMAC */
#endif /* err.h */
