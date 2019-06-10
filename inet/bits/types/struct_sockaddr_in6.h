#ifndef __struct_sockaddr_in6_defined
#define __struct_sockaddr_in6_defined 1

#include <bits/types.h>
#include <bits/types/struct_in6_addr.h>
#include <bits/types/in_port_t.h>
#include <bits/sockaddr.h>
#include <bits/uapi-compat.h>

/* Kernel headers may already have defined this type.  */
#if !defined __UAPI_DEF_SOCKADDR_IN6 || __UAPI_DEF_SOCKADDR_IN6 == 0
#define __UAPI_DEF_SOCKADDR_IN6 0

/* Structure describing an IPv6 socket address.  */
struct sockaddr_in6
{
  __SOCKADDR_COMMON (sin6_);
  in_port_t sin6_port;		/* Transport layer port # */
  uint32_t sin6_flowinfo;	/* IPv6 flow information */
  struct in6_addr sin6_addr;	/* IPv6 address */
  uint32_t sin6_scope_id;	/* IPv6 scope-id */
};

#endif /* __UAPI_DEF_SOCKADDR_IN6 is zero or not defined.  */
#endif /* struct_sockaddr_in6.h.  */
