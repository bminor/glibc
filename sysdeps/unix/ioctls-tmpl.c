#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>

#ifdef	SIOCGIFCONF
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/route.h>
#if	defined(SIOCGARP) && !defined(ARPOP_REQUEST)
#include <net/if_arp.h>
#endif
#ifdef	SIOCGNIT
#include <net/nit.h>
#endif
#endif

/* These exist on Sequents.  */
#ifdef SMIOSTATS
#include <sec/sec.h>
#include <sec/sm.h>
#endif
#ifdef SMIOGETREBOOT0
#include <i386/cfg.h>
#endif
#ifdef ZIOCBCMD
#include <zdc/zdc.h>
#endif

#define	DEFINE(name, value) \
  printf("#define %s 0x%.8x\n", (name), (value))

int
main()
{
  REQUESTS

  exit(0);
  return 0;
}
