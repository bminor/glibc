/* Copyright (C) 1990 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef	_NETINET_IN_H

#define	_NETINET_IN_H	1
#include <features.h>

#include <sys/socket.h>


/* Standard well-defined IP protocols.  */
enum
  {
    IPPROTO_IP = 0,	/* Dummy protocol for TCP.  */
    IPPROTO_ICMP = 1,	/* Internet Control Message Protocol.  */
    IPPROTO_GGP = 2,	/* Gateway Protocol (deprecated).  */
    IPPROTO_TCP = 6,	/* Transmission Control Protocol.  */
    IPPROTO_EGP = 8,	/* Exterior Gateway Protocol.  */
    IPPROTO_PUP = 12,	/* PUP protocol.  */
    IPPROTO_UDP = 17,	/* User Datagram Protocol.  */
    IPPROTO_IDP = 22,	/* XNS IDP protocol.  */

    IPPROTO_RAW = 255,	/* Raw IP packets.  */
    IPPROTO_MAX,
  };

/* Standard well-known ports.  */
enum
  {
    IPPORT_ECHO = 7,		/* Echo service.  */
    IPPORT_DISCARD = 9,		/* Discard transmissions service.  */
    IPPORT_SYSTAT = 11,		/* System status service.  */
    IPPORT_DAYTIME = 13,	/* Time of day service.  */
    IPPORT_NETSTAT = 15,	/* Network status service.  */
    IPPORT_FTP = 21,		/* File Transfer Protocol.  */
    IPPORT_TELNET = 23,		/* Telnet protocol.  */
    IPPORT_SMTP = 25,		/* Simple Mail Transfer Protocol.  */
    IPPORT_TIMESERVER = 37,	/* Timeserver service.  */
    IPPORT_NAMESERVER = 42,	/* Domain Name Service.  */
    IPPROT_WHOIS = 43,		/* Internet Whois service.  */
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,		/* Trivial File Transfer Protocol.  */
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,		/* Finger service.  */
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,		/* SUPDUP protocol.  */


    IPPORT_EXECSERVER = 512,	/* execd service.  */
    IPPORT_LOGINSERVER = 513,	/* rlogind service.  */
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,

    /* UDP ports.  */
    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,

    /* Ports less than this value are reserved for privileged processes.  */
    IPPORT_RESERVED = 1024,

    /* Ports greater this value are reserved for (non-privileged) servers.  */
    IPPORT_USERRESERVED = 5000,
  };


/* Link numbers.  */
#define	IMPLINK_IP		155
#define	IMPLINK_LOWEXPER	156
#define	IMPLINK_HIGHEXPER	158


/* Internet address.  */
struct in_addr
  {
    unsigned long int s_addr;
  };


/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */

#define	IN_CLASSA(a)		((((long int) (a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		(0xffffffff & ~IN_CLASSA_NET)
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(a)		((((long int) (a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		(0xffffffff & ~IN_CLASSB_NET)
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(a)		((((long int) (a)) & 0xc0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		(0xffffffff & ~IN_CLASSC_NET)

#define	IN_CLASSD(a)		((((long int) (a)) & 0xf0000000) = 0xe0000000)
#define	IN_MULTICAST(a)		IN_CLASSD(a)

#define	IN_EXPERIMENTAL(a)	((((long int) (a)) & 0xe0000000) = 0xe0000000)
#define	IN_BADCLASS(a)		((((long int) (a)) & 0xf0000000) = 0xf0000000)

/* Address to accept any incoming messages.  */
#define	INADDR_ANY		((unsigned long int) 0x00000000)
/* Address to send to all hosts.  */
#define	INADDR_BROADCAST	((unsigned long int) 0xffffffff)
/* Address indicating an error return.  */
#define	INADDR_NONE		0xffffffff

/* Network number for local host loopback.  */
#define	IN_LOOPBACKNET	127
/* Address to loopback in software to local host.  */
#define	INADDR_LOOPBACK	0x7f000001	/* Internet address 127.0.0.1.  */


/* Structure describing an Internet socket address.  */
struct sockaddr_in
  {
    short int sin_family;		/* Address family.  */
    unsigned short int sin_port;	/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char __pad[sizeof(struct sockaddr) - sizeof(short int) -
			sizeof(unsigned short int) - sizeof(struct in_addr)];
  };


/* Options for use with `getsockopt' and `setsockopt' at the IP level.  */
#define	IP_OPTIONS	1	/* IP per-packet options.  */
#define	IP_HDRINCL	2	/* Raw packet header option.  */


/* Get number representation conversion macros.  */
#include <cvt.h>

#endif	/* netinet/in.h */
