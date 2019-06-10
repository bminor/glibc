#ifndef __struct_sockaddr_in_defined
#define __struct_sockaddr_in_defined 1

#include <bits/types.h>
#include <bits/types/struct_in_addr.h>
#include <bits/types/in_port_t.h>
#include <bits/sockaddr.h>
#include <bits/uapi-compat.h>

/* Kernel headers may already have defined this type.  */
#if !defined __UAPI_DEF_SOCKADDR_IN || __UAPI_DEF_SOCKADDR_IN == 0
#define __UAPI_DEF_SOCKADDR_IN 0

/* Structure describing an IPv4 socket address.  */
struct sockaddr_in
{
  __SOCKADDR_COMMON (sin_);
  in_port_t sin_port;			/* Port number.  */
  struct in_addr sin_addr;		/* IPv4 address.  */

  /* Pad to size of `struct sockaddr'.  */
  unsigned char sin_zero[__SOCKADDR_DATA_SIZE
                         - sizeof (in_port_t)
                         - sizeof (struct in_addr)];
};

#endif /* __UAPI_DEF_SOCKADDR_IN is zero or not defined.  */
#endif /* struct_sockaddr_in.h.  */
