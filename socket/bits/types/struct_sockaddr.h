#ifndef __struct_sockaddr_defined
#define __struct_sockaddr_defined 1

#include <bits/sockaddr.h>

/* Structure describing a generic socket address.  For historical
   reasons this type is smaller than many address families' concrete
   socket addresses.  You may want struct sockaddr_storage instead.  */
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: family and perhaps length.  */
    char sa_data[__SOCKADDR_DATA_SIZE];	/* Address data.  */
  };

#endif
