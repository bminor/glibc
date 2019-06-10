#ifndef __struct_in_addr_defined
#define __struct_in_addr_defined 1

#include <bits/types/in_addr_t.h>
#include <bits/uapi-compat.h>

/* Kernel headers may already have defined this type.  */
#if !defined __UAPI_DEF_IN_ADDR || __UAPI_DEF_IN_ADDR == 0
#define __UAPI_DEF_IN_ADDR 0

/* Struct representing an IPv4 address.  */
struct in_addr
{
  in_addr_t s_addr;
};

#endif /* __UAPI_DEF_IN_ADDR is zero or not defined. */
#endif /* struct_in_addr.h.  */
