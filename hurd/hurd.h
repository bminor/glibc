/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

#ifndef	_HURD_H

#define	_HURD_H	1
#include <features.h>


/* Get types, macros, constants and function declarations
   for all Mach microkernel interaction.  */
#include <mach.h>
#include <mach/mig_errors.h>

/* Get types and constants necessary for Hurd interfaces.  */
#include <hurd/hurd_types.h>

/* Get MiG stub declarations for commonly used Hurd interfaces.  */
#include <hurd/auth.h>
#include <hurd/process.h>
#include <hurd/fs.h>
#include <hurd/io.h>
#include <hurd/msg.h>

#include <errno.h>
#define	__hurd_fail(err)	(errno = (err), -1)

#define __spin_lock(lockaddr) /* no-op XXX */
#define __spin_unlock(lockaddr) /* no-op XXX */

#define __mutex_lock(lockaddr) /* no-op XXX */
#define __mutex_unlock(lockaddr) /* no-op XXX */


/* Basic ports and info, initialized by startup.  */
extern struct _hurd_port *_hurd_ports;
extern unsigned int _hurd_nports;
extern volatile mode_t _hurd_umask;

/* Shorthand macro for referencing _hurd_ports.  */
#define	__USEPORT(which, expr) \
  _HURD_PORT_USE (&_hurd_ports[INIT_PORT_##which], (expr))

/* Base address and size of the initial stack set up by the exec server.
   If using cthreads, this stack is deallocated in startup.
   Not locked.  */
extern vm_address_t _hurd_stack_base;
extern vm_size_t _hurd_stack_size;

extern thread_t _hurd_msgport_thread;
extern mach_port_t _hurd_msgport; /* Locked by _hurd_siglock.  */

/* Not locked.  If we are using a real dtable, these are turned into that
   and then cleared at startup.  If not, these are never changed after
   startup.  */
extern mach_port_t *_hurd_init_dtable;
extern mach_msg_type_number_t _hurd_init_dtablesize;

/* Return the socket server for sockaddr domain DOMAIN.  */
extern socket_t _hurd_socket_server (int domain);


/* Current process IDs.  */
extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;
#ifdef noteven
extern struct mutex _hurd_pid_lock; /* Locks above.  */
#endif


/* User and group IDs.  */
struct _hurd_id_data
  {
#ifdef noteven
    mutex_t lock;
#endif

    int valid;			/* If following data are up to date.  */

    struct
      {
	uid_t *uids;
	gid_t *gids;
	unsigned int nuids, ngids;
      } gen, aux;

    auth_t rid_auth;		/* Cache used by access.  */
  };
extern struct _hurd_id_data _hurd_id;
/* Update _hurd_id (caller should be holding the lock).  */
extern error_t _hurd_check_ids (void);


/* Unix `data break', for brk and sbrk.
   If brk and sbrk are not used, this info will not be initialized or used.  */
extern vm_address_t _hurd_brk;	/* Data break.  */
extern vm_address_t _hurd_data_end; /* End of allocated space.  */
#ifdef noteven
extern struct mutex _hurd_brk_lock; /* Locks brk and data_end.  */
#endif
extern int _hurd_set_data_limit (const struct rlimit *);

/* Set the data break; the brk lock must
   be held, and is released on return.  */
extern int _hurd_set_brk (vm_address_t newbrk);

/* Resource limit on core file size.  Enforced by hurdsig.c.  */
extern int _hurd_core_limit;

/* Calls to get and set basic ports.  */
extern process_t getproc (void);
extern file_t getccdir (void), getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern int setproc (process_t);
extern int setcwdir (file_t), setcrdir (file_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);


extern error_t __hurd_path_split (file_t crdir, file_t cwdir,
				  const char *file,
				  file_t *dir, char **name);
extern error_t hurd_path_split (file_t crdir, file_t cwdir,
				const char *file,
				file_t *dir, char **name);
extern error_t __hurd_path_lookup (file_t crdir, file_t cwdir,
				   const char *file,
				   int flags, mode_t mode,
				   file_t *port);
extern error_t hurd_path_lookup (file_t crdir, file_t cwdir,
				 const char *filename,
				 int flags, mode_t mode,
				 file_t *port);

/* Returns a port to the directory, and sets *NAME to the file name.  */
extern file_t __path_split (const char *file, char **name);
extern file_t path_split (const char *file, char **name);

/* Looks up FILE with the given FLAGS and MODE (as for dir_pathtrans).  */
extern file_t __path_lookup (const char *file, int flags, mode_t mode);
extern file_t path_lookup (const char *file, int flags, mode_t mode);

/* Open a file descriptor on a port.  */
extern int openport (io_t port, int flags);

/* Execute a file, replacing the current program image.  */
extern error_t _hurd_exec (file_t file,
			   char *const argv[],
			   char *const envp[]);

/* Inform the proc server we have exitted with STATUS, and kill the
   task thoroughly.  This function never returns, no matter what.  */
extern volatile void _hurd_exit (int status);

/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.
   Then vm_deallocate PORTARRAY and INTARRAY.  */
extern void _hurd_init (int flags, char **argv,
			mach_port_t *portarray, size_t portarraysize,
			int *intarray, size_t intarraysize);

/* Do startup handshaking with the proc server.  */
extern void _hurd_proc_init (char **argv);

/* Fetch the host privileged port and device master port from the proc
   server.  They are fetched only once and then cached in the variables
   below.  A special program that gets them from somewhere other than the
   proc server (such as a bootstrap filesystem) can set these variables at
   startup to install the ports.  */
extern kern_return_t get_privileged_ports (host_priv_t *host_priv_ptr,
					   device_t *device_master_ptr);
extern mach_port_t _hurd_host_priv, _hurd_device_master;

/* Convert between PIDs and task ports.  */
extern pid_t __task2pid (task_t), task2pid (task_t);
extern task_t __pid2task (pid_t), pid2task (pid_t);


#endif	/* hurd.h */
