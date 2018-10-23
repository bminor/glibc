/* Although ARC CPU can do BE, it is not supported.  */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#ifdef __LITTLE_ENDIAN__
# define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#ifdef __BIG_ENDIAN__
# error "Big Endian NOt supported on ARC"
#endif
