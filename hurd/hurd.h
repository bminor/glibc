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

#define __spin_lock(lockaddr) /* no-op XXX */
#define __spin_unlock(lockaddr) /* no-op XXX */

#define __mutex_lock(lockaddr) /* no-op XXX */
#define __mutex_unlock(lockaddr) /* no-op XXX */

/* Get `struct hurd_port' and related definitions implementing lightweight
   user references for ports.  These are used pervasively throughout the C
   library; this is here to avoid putting it in nearly every source file.  */
#include <hurd/port.h>

#include <errno.h>
#define	__hurd_fail(err)	(errno = (err), -1)

/* Basic ports and info, initialized by startup.  */

extern struct hurd_port *_hurd_ports;
extern unsigned int _hurd_nports;
extern volatile mode_t _hurd_umask;

/* Shorthand macro for referencing _hurd_ports (see <hurd/port.h>).  */

#define	__USEPORT(which, expr) \
  HURD_PORT_USE (&_hurd_ports[INIT_PORT_##which], (expr))


/* Base address and size of the initial stack set up by the exec server.
   If using cthreads, this stack is deallocated in startup.
   Not locked.  */

extern vm_address_t _hurd_stack_base;
extern vm_size_t _hurd_stack_size;

/* Initial file descriptor table we were passed at startup.  If we are
   using a real dtable, these are turned into that and then cleared at
   startup.  If not, these are never changed after startup.  Not locked.  */

extern mach_port_t *_hurd_init_dtable;
extern mach_msg_type_number_t _hurd_init_dtablesize;

/* Miscellaneous library state.  */


/* Current process IDs.  */

extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;
#ifdef noteven
extern struct mutex _hurd_pid_lock; /* Locks above.  */
#endif

/* Unix `data break', for brk and sbrk.
   If brk and sbrk are not used, this info will not be initialized or used.  */


/* Data break.  This is what `sbrk (0)' returns.  */

extern vm_address_t _hurd_brk;

/* End of allocated space.  This is generally `round_page (_hurd_brk)'.  */

extern vm_address_t _hurd_data_end;

/* This mutex locks _hurd_brk and _hurd_data_end.  */

#ifdef noteven
extern struct mutex _hurd_brk_lock;
#endif

/* Set the data resource limit (RLIM_DATA).  */

extern int _hurd_set_data_limit (const struct rlimit *);

/* Set the data break to NEWBRK; _hurd_brk_lock must
   be held, and is released on return.  */

extern int _hurd_set_brk (vm_address_t newbrk);

/* Calls to get and set basic ports.  */

extern process_t getproc (void);
extern file_t getccdir (void), getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern int setproc (process_t);
extern int setcwdir (file_t), setcrdir (file_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);


/* Split FILE into a directory and a name within the directory.  Look up a
   port for the directory and store it in *DIR; store in *NAME a pointer
   into FILE where the name within directory begins.  The directory lookup
   uses CRDIR for the root directory and CWDIR for the current directory.
   Returns zero on success or an error code.  */

extern error_t __hurd_path_split (file_t crdir, file_t cwdir,
				  const char *file,
				  file_t *dir, char **name);
extern error_t hurd_path_split (file_t crdir, file_t cwdir,
				const char *file,
				file_t *dir, char **name);

/* Open a port to FILE with the given FLAGS and MODE (see <fcntl.h>).
   The file lookup uses CRDIR for the root directory and CWDIR for the
   current directory.  If successful, returns zero and store the port
   to FILE in *PORT; otherwise returns an error code. */

extern error_t __hurd_path_lookup (file_t crdir, file_t cwdir,
				   const char *file,
				   int flags, mode_t mode,
				   file_t *port);
extern error_t hurd_path_lookup (file_t crdir, file_t cwdir,
				 const char *filename,
				 int flags, mode_t mode,
				 file_t *port);

/* Split FILE into a directory and a name within the directory.  The
   directory lookup uses the current root and working directory.  If
   successful, stores in *NAME a pointer into FILE where the name
   within directory begins and returns a port to the directory;
   otherwise sets `errno' and returns MACH_PORT_NULL.  */

extern file_t __path_split (const char *file, char **name);
extern file_t path_split (const char *file, char **name);

/* Open a port to FILE with the given FLAGS and MODE (see <fcntl.h>).
   The file lookup uses the current root and working directory.
   Returns a port to the file if successful; otherwise sets `errno'
   and returns MACH_PORT_NULL.  */

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

extern void _hurd_exit (int status) __attribute__ ((noreturn));


/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.
   Then vm_deallocate PORTARRAY and INTARRAY.  */

extern void _hurd_init (int flags, char **argv,
			mach_port_t *portarray, size_t portarraysize,
			int *intarray, size_t intarraysize);

/* Do startup handshaking with the proc server.  */

extern void _hurd_proc_init (char **argv);

/* Return the socket server for sockaddr domain DOMAIN.  */

extern socket_t _hurd_socket_server (int domain);

/* Fetch the host privileged port and device master port from the proc
   server.  They are fetched only once and then cached in the
   variables below.  A special program that gets them from somewhere
   other than the proc server (such as a bootstrap filesystem) can set
   these variables to install the ports.  */

extern kern_return_t get_privileged_ports (host_priv_t *host_priv_ptr,
					   device_t *device_master_ptr);
extern mach_port_t _hurd_host_priv, _hurd_device_master;

/* Return the PID of the task whose control port is TASK.
   On error, sets `errno' and returns -1.  */

extern pid_t __task2pid (task_t task), task2pid (task_t task);

/* Return the task control port of process PID.
   On error, sets `errno' and returns MACH_PORT_NULL.  */

extern task_t __pid2task (pid_t pid), pid2task (pid_t pid);


/* Return the io server port for file descriptor FD.
   This adds a Mach user reference to the returned port.
   On error, sets `errno' and returns MACH_PORT_NULL.  */

extern io_t __getdport (int fd), getdport (int fd);



#endif	/* hurd.h */
