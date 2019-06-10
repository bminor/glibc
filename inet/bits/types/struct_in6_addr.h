#ifndef __struct_in6_addr_defined
#define __struct_in6_addr_defined 1

#include <features.h>
#include <bits/types.h>
#include <bits/uapi-compat.h>

/* Kernel headers may already have defined this type.  */
#if !defined __UAPI_DEF_IN6_ADDR || __UAPI_DEF_IN6_ADDR == 0
#define __UAPI_DEF_IN6_ADDR 0
#define __UAPI_DEF_IN6_ADDR_ALT 0

/* Struct representing an IPv6 address.  */
struct in6_addr
{
  union
  {
    __uint8_t  __u6_addr8[16];
    __uint16_t __u6_addr16[8];
    __uint32_t __u6_addr32[4];
  } __in6_u;
#define s6_addr			__in6_u.__u6_addr8
#ifdef __USE_MISC
# define s6_addr16		__in6_u.__u6_addr16
# define s6_addr32		__in6_u.__u6_addr32
#endif
};

#endif /* __UAPI_DEF_IN6_ADDR is zero or not defined.  */
#endif /* struct_in6_addr.h.  */
