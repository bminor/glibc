#ifndef __struct_sockaddr_storage_defined
#define __struct_sockaddr_storage_defined 1

#include <bits/sockaddr.h>

/* Structure large enough to hold any socket address (with the historical
   exception of AF_UNIX).  */
#ifndef __ss_aligntype
# define __ss_aligntype	unsigned long int
#endif
#define _SS_PADSIZE \
  (_SS_SIZE - __SOCKADDR_COMMON_SIZE - sizeof (__ss_aligntype))

struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);	/* Address family, etc.  */
    char __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align;	/* Force desired alignment.  */
  };

#endif /* bits/types/struct_sockaddr_storage.h */
