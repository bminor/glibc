/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SYS_SOCKET_H

#define	_SYS_SOCKET_H	1
#include <features.h>

__BEGIN_DECLS

#define	__need_size_t
#include <stddef.h>


/* Types of sockets.  */
enum __socket_type
{
  SOCK_STREAM = 1,		/* Sequenced, reliable, connection-based
				   byte streams.  */
  SOCK_DGRAM = 2,		/* Connectionless, unreliable datagrams
				   of fixed maximum length.  */
  SOCK_RAW = 3,			/* Raw protocol interface.  */
  SOCK_RDM = 4,			/* Reliably-delivered messages.  */
  SOCK_SEQPACKET = 5,		/* Sequenced, reliable, connection-based,
				   datagrams of fixed maximum length.  */
};

/* Protocol families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define	PF_FILE		1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_FILE	/* BSD name for PF_FILE.  */
#define	PF_INET		2	/* IP protocol family.  */
#define	PF_IMPLINK	3	/* ARPAnet IMP protocol.  */
#define	PF_PUP		4	/* PUP protocols.  */
#define	PF_CHAOS	5	/* MIT Chaos protocols.  */
#define	PF_NS		6	/* Xerox NS protocols.  */
#define	PF_ISO		7	/* ISO protocols.  */
#define	PF_OSI		PF_ISO
#define	PF_ECMA		8	/* ECMA protocols.  */
#define	PF_DATAKIT	9	/* AT&T Datakit protocols.  */
#define	PF_CCITT	10	/* CCITT protocols (X.25 et al).  */
#define	PF_SNA		11	/* IBM SNA protocol.  */
#define	PF_DECnet	12	/* DECnet protocols.  */
#define	PF_DLI		13	/* Direct data link interface.  */
#define	PF_LAT		14	/* DEC Local Area Transport protocol.  */
#define	PF_HYLINK	15	/* NSC Hyperchannel protocol.  */
#define	PF_APPLETALK	16	/* Don't use this.  */
#define	PF_ROUTE	17	/* Internal Routing Protocol.  */
#define	PF_LINK		18	/* Link layer interface.  */
#define	PF_MAX		19

/* Address families.  */
#define	AF_UNSPEC	PF_UNSPEC
#define	AF_FILE		PF_FILE
#define	AF_UNIX		PF_UNIX
#define	AF_INET		PF_INET
#define	AF_IMPLINK	PF_IMPLINK
#define	AF_PUP		PF_PUP
#define	AF_CHAOS	PF_CHAOS
#define	AF_NS		PF_NS
#define	AF_ISO		PF_ISO
#define	AF_OSI		PF_OSI
#define	AF_ECMA		PF_ECMA
#define	AF_DATAKIT	PF_DATAKIT
#define	AF_CCITT	PF_CCITT
#define	AF_SNA		PF_SNA
#define	AF_DECnet	PF_DECnet
#define	AF_DLI		PF_DLI
#define	AF_LAT		PF_LAT
#define	AF_HYLINK	PF_HYLINK
#define	AF_APPLETALK	PF_APPLETALK
#define	AF_ROUTE	PF_ROUTE
#define	AF_LINK		PF_LINK
#define	AF_MAX		PF_MAX


/* Structure describing a generic socket address.  */
struct sockaddr
  {
    unsigned short int sa_family; /* Address family.  */
    char sa_data[14];		/* Address data.  */
  };


/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or -1 for errors.  */
extern int socket __P ((int __domain, enum __socket_type __type,
			int __protocol));

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
extern int socketpair __P ((int __domain, enum __socket_type __type,
			    int __protocol, int __fds[2]));

/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
extern int bind __P ((int __fd, struct sockaddr * __addr, size_t __len));

/* Put the local address of FD into *ADDR and its length in *LEN.  */
extern int getsockname __P ((int __fd, struct sockaddr * __addr,
			     size_t * __len));

/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.  */
extern int connect __P ((int __fd, struct sockaddr * __addr, size_t __len));

/* Put the address of the peer connected to socket FD into *ADDR
   (which is *LEN bytes long), and its actual length into *LEN.  */
extern int getpeername __P ((int __fd, struct sockaddr * __addr,
			     size_t * __len));


/* Bits in the FLAGS argument to `send', `recv', et al.  */
enum
  {
    MSG_OOB = 1,		/* Process out-of-band data.  */
    MSG_PEEK = 2,		/* Peek at incoming messages.  */
    MSG_DONTROUTE = 4,		/* Don't use local routing.  */
  };

/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.  */
extern int send __P ((int __fd, __ptr_t __buf, size_t __n, int __flags));

/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.  */
extern int recv __P ((int __fd, __ptr_t __buf, size_t __n, int __flags));

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.  */
extern int sendto __P ((int __fd, __ptr_t __buf, size_t __n, int __flags,
			struct sockaddr * __addr, size_t __addr_len));

/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or -1 for errors.  */
extern int recvfrom __P ((int __fd, __ptr_t __buf, size_t __n, int __flags,
			  struct sockaddr * __addr, size_t * __addr_len));



/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
  {
    __ptr_t msg_name;		/* Address to send to/receive from.  */
    size_t msg_namelen;		/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    size_t msg_iovlen;		/* Number of elements in the vector.  */

    __ptr_t msg_accrights;	/* Access rights information.  */
    size_t msg_accrightslen;	/* Length of access rights information.  */
  };

/* Send a message described MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.  */
extern int sendmsg __P ((int __fd, __const struct msghdr * __message,
			 int __flags));

/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.  */
extern int recvmsg __P ((int __fd, struct msghdr * __message, int __flags));


/* Protocol number used to manipulate socket-level options
   with `getsockopt' and `setsockopt'.  */
#define	SOL_SOCKET	0xffff

/* Socket-level options for `getsockopt' and `setsockopt'.  */
enum
  {
    SO_DEBUG = 0x0001,		/* Record debugging information.  */
    SO_ACCEPTCONN = 0x0002,	/* Accept connections on socket.  */
    SO_REUSEADDR = 0x0004,	/* Allow reuse of local addresses.  */
    SO_KEEPALIVE = 0x0008,	/* Keep connections alive and send
				   SIGPIPE when they die.  */
    SO_DONTROUTE = 0x0010,	/* Don't do local routing.  */
    SO_BROADCAST = 0x0020,	/* Allow transmission of
				   broadcast messages.  */
    SO_USELOOPBACK = 0x0040,	/* Use the software loopback to avoid
				   hardware use when possible.  */
    SO_LINGER = 0x0080,		/* Block on close of a reliable
				   socket to transmit pending data.  */
    SO_OOBINLINE = 0x0100,	/* Receive out-of-band data in-band.  */

    SO_SNDBUF = 0x1001,		/* Send buffer size.  */
    SO_RCVBUF = 0x1002,		/* Receive buffer.  */
    SO_SNDLOWAT = 0x1003,	/* Send low-water mark.  */
    SO_RCVLOWAT = 0x1004,	/* Receive low-water mark.  */
    SO_SNDTIMEO = 0x1005,	/* Send timeout.  */
    SO_RCVTIMEO = 0x1006,	/* Receive timeout.  */

    SO_ERROR = 0x1007,		/* Get and clear error status.  */
    SO_STYLE = 0x1008,		/* Get socket connection style.  */
    SO_TYPE = SO_STYLE,		/* Compatible name for SO_STYLE.  */
  };

/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };


/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
extern int getsockopt __P ((int __fd, int __level, int __optname,
			    __ptr_t __optval, size_t * __optlen));

/* Set socket FD's option OPTNAME at protocol level LEVEL
   to *OPTVAL (which is OPTLEN bytes long).
   Returns 0 on success, -1 for errors.  */
extern int setsockopt __P ((int __fd, int __level, int __optname,
			    __ptr_t __optval, size_t __optlen));


/* Prepare to accept connections on socket FD.
   N connection requests will be queued before further requests are refused.
   Returns 0 on success, -1 for errors.  */
extern int listen __P ((int __fd, unsigned int __n));

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.  */
extern int accept __P ((int __fd, struct sockaddr * __addr,
			size_t * __addr_len));

/* Shut down all or part of the connection open on socket FD.
   HOW determines what to shut down:
     0 = No more receptions;
     1 = No more transmissions;
     2 = No more receptions or transmissions.
   Returns 0 on success, -1 for errors.  */
extern int shutdown __P ((int __fd, int __how));


__END_DECLS

#endif /* sys/socket.h */
