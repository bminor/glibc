/* Lightweight user references for ports.
Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_HURD_PORT_H

#define	_HURD_PORT_H	1
#include <features.h>

#include <mach/mach_port.h>


/* Structure describing a cell containing a port.  With the lock held, a
   user extracts PORT, and attaches his own link (in local storage) to the
   USERS chain.  PORT can then safely be used.  When PORT is no longer
   needed, with the lock held, the user removes his link from the chain.
   If his link is the last, and PORT has changed since he fetched it, the
   user deallocates the port he used.  */
struct hurd_port
  {
#ifdef noteven
    spin_lock_t lock;		/* Locks rest.  */
#endif
    struct hurd_port_userlink *users; /* Chain of users; see below.  */
    mach_port_t port;		/* Port. */
  };

/* This structure is simply a doubly-linked list.
   Users of a port cell are recorded by their presence in the list.  */   
struct hurd_port_userlink
  {
    struct hurd_port_userlink *next, **prevp;
  };

/* Evaluate EXPR with the variable `port' bound to the port in PORTCELL.  */
#define	HURD_PORT_USE(portcell, expr)					      \
  ({ struct _hurd_port *const __p = (portcell);				      \
     struct _hurd_port_userlink __link;					      \
     const mach_port_t port = _hurd_port_get (__p, &__link);		      \
     __typeof(expr) __result = (expr);					      \
     _hurd_port_free (__p, &__link, port);				      \
     __result; })

/* Initialize *PORT to INIT.  */
extern inline void
_hurd_port_init (struct hurd_port *port, mach_port_t init)
{
#ifdef noteven
  __spin_lock_init (&port->lock);
#endif
  port->users = NULL;
  port->port = init;
}

/* Get a reference to *PORT, which is locked.
   Pass return value and LINK to _hurd_port_free when done.  */
extern inline mach_port_t
_hurd_port_locked_get (struct hurd_port *port,
		       struct hurd_port_userlink *link)
{
  mach_port_t result;
  result = port->port;
  if (result != MACH_PORT_NULL)
    {
      link->next = port->users;
      if (link->next)
	link->next->prevp = &link->next;
      link->prevp = &port->users;
      port->users = link;
    }
  __spin_unlock (&port->lock);
  return result;
}

/* Same, but locks PORT first.  */
extern inline mach_port_t
_hurd_port_get (struct hurd_port *port,
		struct hurd_port_userlink *link)
{
  __spin_lock (&port->lock);
  return _hurd_port_locked_get (port, link);
}

/* Free a reference gotten with
   `USED_PORT = _hurd_port_get (PORT, LINK);' */
extern inline void
_hurd_port_free (struct hurd_port *port,
		 struct hurd_port_userlink *link,
		 mach_port_t used_port)
{
  int dealloc;
  __spin_lock (&port->lock);
  /* We should deallocate USED_PORT if our chain has been detached from the
     cell (and thus has a nil `prevp'), and there is no next link
     representing another user reference to the same port we fetched.  */
  dealloc = ! link->next && ! link->prevp;
  /* Remove our link from the chain of current users.  */
  if (link->prevp)
    *link->prevp = link->next;
  if (link->next)
    link->next->prevp = link->prevp;
  __spin_unlock (&port->lock);
  if (dealloc)
    __mach_port_deallocate (__mach_task_self (), used_port);
}

/* Set *PORT's port to NEWPORT.  NEWPORT's reference is consumed by PORT->port.
   PORT->lock is locked.  */
extern inline void
_hurd_port_locked_set (struct hurd_port *port, mach_port_t newport)
{
  mach_port_t old;
  if (port->users == NULL)
    old = port->port;
  else
    {
      old = MACH_PORT_NULL;
      /* Detach the chain of current users from the cell.  The last user to
	 remove his link from that chain will deallocate the old port.  */
      port->users->prevp = NULL;
      port->users = NULL;
    }
  port->port = newport;
  __spin_unlock (&port->lock);
  if (old != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), old);
}

/* Same, but locks PORT first.  */
extern inline void
_hurd_port_set (struct hurd_port *port, mach_port_t newport)
{
  __spin_lock (&port->lock);
  return _hurd_port_locked_set (port, newport);
}


#endif	/* hurd/port.h */
