/* Copyright (C) 1991 Free Software Foundation, Inc.
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

/*
 *	POSIX Standard: 2.10 Symbolic Constants		<unistd.h>
 */

#ifndef	_UNISTD_H

#define	_UNISTD_H	1
#include <features.h>

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from sysconf.  */

/* POSIX Standard approved as IEEE Std 1003.1 as of August, 1988.  */
#define	_POSIX_VERSION	199009L

#ifdef	__USE_POSIX2
#define	_POSIX2_C_VERSION	199912L	/* Invalid until 1003.2 is done.  */

/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define	_POSIX2_C_BIND	1

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define	_POSIX2_C_DEV	1

/* If defined, the implementation supports the
   FORTRAN Language Development Utilities Option.  */
#define	_POSIX2_FORT_DEV	1

/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define	_POSIX2_SW_DEV	1
#endif


/* Get values of POSIX options:

   If these symbols are defined, the corresponding features are
   always available.  If not, they may be available sometimes.
   The current values can be obtained with `sysconf'.

   _POSIX_JOB_CONTROL	Job control is supported.
   _POSIX_SAVED_IDS	Processes have a saved set-user-ID
   			and a saved set-group-ID.

   If any of these symbols is defined as -1, the corresponding option is not
   true for any file.  If any is defined as other than -1, the corresponding
   option is true for all files.  If a symbol is not defined at all, the value
   for a specific file can be obtained from `pathconf' and `fpathconf'.

   _POSIX_CHOWN_RESTRICTED	Only the super user can use `chown' to change
   				the owner of a file.  `chown' can only be used
				to change the group ID of a file to a group of
				which the calling process is a member.
   _POSIX_NO_TRUNC		Pathname components longer than
   				NAME_MAX generate an error.
   _POSIX_VDISABLE		If defined, if the value of an element of the
				`c_cc' member of `struct termios' is
				_POSIX_VDISABLE, no character will have the
				effect associated with that element.
   */

#include <posix_options.h>


/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */


/* All functions that are not declared anywhere else.  */

#include <gnu/types.h>

#ifndef	ssize_t
#define	ssize_t	__ssize_t
#endif

#define	__need_size_t
#include <stddef.h>


/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4	/* Test for read permission.  */
#define	W_OK	2	/* Test for write permission.  */
#define	X_OK	1	/* Test for execute permission.  */
#define	F_OK	0	/* Test for existence.  */

/* Test for access to NAME.  */
extern int EXFUN(__access, (CONST char *__name, int __type));
extern int EXFUN(access, (CONST char *__name, int __type));

#ifdef	__OPTIMIZE__
#define	access(name, type)	__access((name), (type))
#endif	/* Optimizing.  */


/* Values for the WHENCE argument to lseek.  */
#ifndef	_STDIO_H	/* <stdio.h> has the same definitions.  */
#define	SEEK_SET	0	/* Seek from beginning of file.  */
#define	SEEK_CUR	1	/* Seek from current position.  */
#define	SEEK_END	2	/* Seek from end of file.  */
#endif

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the old file position.  */
extern __off_t EXFUN(__lseek, (int __fd, __off_t __offset, int __whence));
extern __off_t EXFUN(lseek, (int __fd, __off_t __offset, int __whence));

/* Close the file descriptor FD.  */
extern int EXFUN(__close, (int __fd));
extern int EXFUN(close, (int __fd));

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.  */
extern ssize_t EXFUN(__read, (int __fd, PTR __buf, size_t __nbytes));
extern ssize_t EXFUN(read, (int __fd, PTR __buf, size_t __nbytes));

/* Write N bytes of BUF to FD.  Return the number written, or -1.  */
extern ssize_t EXFUN(__write, (int __fd, CONST PTR __buf, size_t __n));
extern ssize_t EXFUN(write, (int __fd, CONST PTR __buf, size_t __n));

#ifdef	__OPTIMIZE__
#define	lseek(fd, offset, whence)	__lseek((fd), (offset), (whence))
#define	close(fd)			__close(fd)
#define	read(fd, buf, n)		__read((fd), (buf), (n))
#define	write(fd, buf, n)		__write((fd), (buf), (n))
#endif	/* Optimizing.  */


/* Create a one-way communication channel (pipe).
   If successul, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
extern int EXFUN(pipe, (int __pipedes[2]));


/* Schedule an alarm.  In SECONDS seconds, the process will get a SIGALRM.
   If SECONDS is zero, any currently scheduled alarm will be cancelled.
   The function returns the number of seconds remaining until the last
   alarm scheduled would have signaled, or zero if there wasn't one.
   There is no return value to indicate an error, but you can set `errno'
   to 0 and check its value after calling `alarm', and this might tell you.
   The signal may come late due to processor scheduling.  */
extern unsigned int EXFUN(alarm, (unsigned int __seconds));

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (thus zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.  */
extern unsigned int EXFUN(sleep, (unsigned int __seconds));


/* Suspend the process until a signal arrives.
   This always returns -1 and sets `errno' to EINTR.  */
extern int EXFUN(pause, (NOARGS));


/* Change the owner and group of FILE.  */
extern int EXFUN(chown, (CONST char *__file AND
			 __uid_t __owner AND __gid_t __group));

#ifdef	__USE_BSD
/* Change the owner and group of the file that FD is open on.  */
extern int EXFUN(fchown, (int __fd AND __uid_t __owner AND __gid_t __group));
#endif

/* Change the process's working directory to PATH.  */
extern int EXFUN(chdir, (CONST char *__path));

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE <= 0, in which case it is as
   big as necessary.  */
extern char *EXFUN(getcwd, (char *__buf, size_t __size));

#ifdef	__USE_BSD
/* Put the absolute pathname of the current working directory in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
extern char *EXFUN(getwd, (char *__buf));
#endif


/* Duplicate FD, returning a new file descriptor on the same file.  */
extern int EXFUN(dup, (int __fd));

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
extern int EXFUN(dup2, (int __fd, int __fd2));


/* NULL-terminated array of "NAME=VALUE" environment variables.  */
extern char **__environ;
extern char **environ;


/* Replace the current process, executing PATH with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
extern int EXFUN(__execve, (CONST char *__path, char *CONST __argv[],
			    char *CONST __envp[]));
extern int EXFUN(execve, (CONST char *__path, char *CONST __argv[],
			  char *CONST __envp[]));

#define	execve	__execve

/* Execute PATH with arguments ARGV and environment from `environ'.  */
extern int EXFUN(execv, (CONST char *__path, char *CONST __argv[]));

#ifdef	__OPTIMIZE__
#define	execv(path, argv)	__execve((path), (argv), __environ)
#endif	/* Optimizing.  */

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
extern int EXFUN(execle, (CONST char *__path, CONST char *__arg, ...));

/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
extern int EXFUN(execl, (CONST char *__path, CONST char *__arg, ...));

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
extern int EXFUN(execvp, (CONST char *__file, char *CONST __argv[]));

/* Execute FILE, searching in the `PATH' environment variable if
   it contains no slashes, with all arguments after FILE until a
   NULL pointer and environment from `environ'.  */
extern int EXFUN(execlp, (CONST char *__file, CONST char *arg, ...));


#ifndef	__NORETURN
#ifdef	__GNUC__
/* The `volatile' keyword tells GCC that a function never returns.  */
#define	__NORETURN	__volatile
#else	/* Not GCC.  */
#define	__NORETURN
#endif	/* GCC.  */
#endif	/* __NORETURN not defined.  */

/* Terminate program execution with the low-order 8 bits of STATUS.  */
extern __NORETURN void EXFUN(_exit, (int __status));


/* Values for the NAME argument to `pathconf' and `fpathconf'.
   These correspond to the _POSIX_* symbols above, but for
   specific files or file descriptors.  */
#define	_PC_LINK_MAX		0
#define	_PC_MAX_CANON		1
#define	_PC_MAX_INPUT		2
#define	_PC_NAME_MAX		3
#define	_PC_PATH_MAX		4
#define	_PC_PIPE_BUF		5
#define	_PC_CHOWN_RESTRICTED	6
#define	_PC_NO_TRUNC		7
#define	_PC_VDISABLE		8

/* Get file-specific configuration information about PATH.  */
extern long int EXFUN(__pathconf, (CONST char *__path, int __name));
extern long int EXFUN(pathconf, (CONST char *__path, int __name));

/* Get file-specific configuration about descriptor FD.  */
extern long int EXFUN(__fpathconf, (int __fd, int __name));
extern long int EXFUN(fpathconf, (int __fd, int __name));

#define	pathconf	__pathconf
#define	fpathconf	__fpathconf


/* Values for the argument to `sysconf'.
   These correspond to the _POSIX_* symbols in <posix_limits.h> and above,
   but may vary at run time.  */
enum
  {
    _SC_ARG_MAX,
    _SC_CHILD_MAX,
    _SC_CLK_TCK,
    _SC_NGROUPS_MAX,
    _SC_OPEN_MAX,
    _SC_STREAM_MAX,
    _SC_TZNAME_MAX,
    _SC_JOB_CONTROL,
    _SC_SAVED_IDS,
    _SC_VERSION,

    /* Values for the argument to `sysconf'
       corresponding to _POSIX2_* symbols.  */
    _SC_BC_BASE_MAX,
    _SC_BC_DIM_MAX,
    _SC_BC_SCALE_MAX,
    _SC_BC_STRING_MAX,
    _SC_EQUIV_CLASS_MAX,
    _SC_EXPR_NEST_MAX,
    _SC_LINE_MAX,
    _SC_RE_DUP_MAX,

    _SC_2_VERSION,
    _SC_2_C_BIND,
    _SC_2_C_DEV,
    _SC_2_FORT_DEV,
    _SC_2_SW_DEV
  };


/* Get the value of the system variable NAME.  */
extern long int EXFUN(sysconf, (int __name));


#ifdef	__USE_POSIX2
/* Values for the argument to `confstr'.  */
#define	_CS_PATH	0	/* The default search path.  */

/* Get the value of the string-valued system variable NAME.  */
extern size_t EXFUN(confstr, (int __name, char *__buf, size_t __len));
#endif


/* Get the process ID of the calling process.  */
extern __pid_t EXFUN(__getpid, (NOARGS));
extern __pid_t EXFUN(getpid, (NOARGS));

/* Get the process ID of the calling process's parent.  */
extern __pid_t EXFUN(__getppid, (NOARGS));
extern __pid_t EXFUN(getppid, (NOARGS));

#ifdef	__OPTIMIZE__
#define	getpid()	__getpid()
#define	getppid()	__getppid()
#endif	/* Optimizing.  */

/* Get the process group ID of process PID.  */
extern __pid_t EXFUN(__getpgrp, (__pid_t __pid));

#ifndef	__FAVOR_BSD
/* Get the process group ID of the calling process.  */
extern __pid_t EXFUN(getpgrp, (NOARGS));
#else	/* Favor BSD.  */
#define	getpgrp(pid)	__getpgrp(pid)
#endif

/* Set the process group ID of the process matching PID to PGID.
   If PID is zero, the current process's process group ID is set.
   If PGID is zero, the process ID of the process is used.  */
extern int EXFUN(setpgid, (__pid_t __pid, __pid_t __pgid));

#ifdef	__USE_BSD
/* Set the process group of PID to PGRP.  */
extern int EXFUN(setpgrp, (__pid_t __pid, __pid_t __pgrp));

#ifdef	__OPTIMIZE__
#define	setpgrp(pid, pgrp)	setpgid((pid), (pgrp))
#endif	/* Optimizing.  */
#endif	/* Use BSD.  */

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
extern __pid_t EXFUN(setsid, (NOARGS));


/* Get the real user ID of the calling process.  */
extern __uid_t EXFUN(getuid, (NOARGS));

/* Get the effective user ID of the calling process.  */
extern __uid_t EXFUN(geteuid, (NOARGS));

/* Get the real group ID of the calling process.  */
extern __gid_t EXFUN(getgid, (NOARGS));

/* Get the effective group ID of the calling process.  */
extern __gid_t EXFUN(getegid, (NOARGS));

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int EXFUN(getgroups, (int __size, __gid_t __list[]));

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int EXFUN(setuid, (__uid_t __uid));

#ifdef	__USE_BSD
/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int EXFUN(__setreuid, (__uid_t __ruid, __uid_t __euid));
extern int EXFUN(setreuid, (__uid_t __ruid, __uid_t __euid));

#define	setreuid	__setreuid
#endif	/* Use BSD.  */

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int EXFUN(setgid, (int __gid));

#ifdef	__USE_BSD
/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int EXFUN(__setregid, (int __rgid, int __egid));
extern int EXFUN(setregid, (int __rgid, int __egid));

#ifdef	__OPTIMIZE__
#define	setregid(rgid, egid)	__setregid((rgid), (egid))
#endif	/* Optimizing.  */
#endif	/* Use BSD.  */


/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t EXFUN(__fork, (NOARGS));
extern __pid_t EXFUN(fork, (NOARGS));

#define	fork	__fork

#ifdef	__USE_BSD
/* Clone the calling process, but without copying the whole address space.
   The the calling process is suspended until the the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t EXFUN(__vfork, (NOARGS));
extern __pid_t EXFUN(vfork, (NOARGS));

#define	vfork	__vfork
#endif	/* Use BSD. */


/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
extern char *EXFUN(ttyname, (int __fd));

/* Return 1 if FD is a valid descriptor associated
   with a terminal, zero if not.  */
extern int EXFUN(__isatty, (int __fd));
extern int EXFUN(isatty, (int __fd));

#ifdef	__OPTIMIZE__
#define	isatty(fd)	__isatty(fd)
#endif	/* Optimizing.  */


/* Make a link to FROM named TO.  */
extern int EXFUN(__link, (CONST char *__from, CONST char *__to));
extern int EXFUN(link, (CONST char *__from, CONST char *__to));
#define	link	__link

#ifdef	__USE_BSD
/* Make a symbolic link to FROM named TO.  */
extern int EXFUN(symlink, (CONST char *__from, CONST char *__to));

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
extern int EXFUN(readlink, (CONST char *__path, char *__buf, size_t __len));
#endif	/* Use BSD.  */

/* Remove the link NAME.  */
extern int EXFUN(__unlink, (CONST char *__name));
extern int EXFUN(unlink, (CONST char *__name));

#ifdef	__OPTIMIZE__
#define	unlink(name)	__unlink(name)
#endif	/* Optimizing.  */

/* Remove the directory PATH.  */
extern int EXFUN(rmdir, (CONST char *__path));


/* Return the foreground process group ID of FD.  */
extern __pid_t EXFUN(tcgetpgrp, (int __fd));

/* Set the foreground process group ID of FD set PGRP_ID.  */
extern int EXFUN(tcsetpgrp, (int __fd, __pid_t __pgrp_id));


/* Return the login name of the user.  */
extern char *EXFUN(getlogin, (NOARGS));

#ifdef	__USE_BSD
/* Set the login name returned by `getlogin'.  */
extern int EXFUN(setlogin, (CONST char *__name));
#endif


#ifdef	__USE_POSIX2
/* Process the arguments in ARGV (ARGC of them, minus
   the program name) for options given in OPTS.

   If `opterr' is zero, no messages are generated
   for invalid options; it defaults to 1.
   `optind' is the current index into ARGV.
   `optarg' is the argument corresponding to the current option.
   Return the option character from OPTS just read.
   Return -1 when there are no more options.
   For unrecognized options, or options missing arguments,
   `optopt' is set to the option letter, and '?' is returned.

   The OPTS string is a list of characters which are recognized option
   letters, optionally followed by colons, specifying that that letter
   takes an argument, to be placed in `optarg'.

   If a letter in OPTS is followed by two colons, its argument is optional.
   This behavior is specific to the GNU `getopt'.

   The argument `--' causes premature termination of argument scanning,
   explicitly telling `getopt' that there are no more options.

   If OPTS begins with `--', then non-option arguments
   are treated as arguments to the option '\0'.
   This behavior is specific to the GNU `getopt'.  */
extern int EXFUN(getopt, (int __argc, char **__argv,
			  CONST char *__opts));
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
#endif


#ifdef	__USE_BSD
/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
extern int EXFUN(__gethostname, (char *__name, size_t __len));
extern int EXFUN(gethostname, (char *__name, size_t __len));

#ifdef	__OPTIMIZE__
#define	gethostname(name, len)	__gethostname((name), (len))
#endif

/* Set the name of the current host to NAME, which is LEN bytes long.
   This call is restricted to the super-user.  */
extern int EXFUN(sethostname, (CONST char *__name, size_t __len));

/* Return the current machine's Internet number.  */
extern long int EXFUN(gethostid, (NOARGS));

/* Set the current machine's Internet number to ID.
   This call is restricted to the super-user.  */
extern int EXFUN(sethostid, (long int __id));


/* Return the number of bytes in a page.  This is the system's page size,
   which is not necessarily the same as the hardware page size.  */
extern int EXFUN(__getpagesize, (NOARGS));
extern int EXFUN(getpagesize, (NOARGS));

#ifdef	__OPTIMIZE__
#define	getpagesize()	__getpagesize()
#endif	/* Optimizing.  */


/* Return the maximum number of file descriptors
   the current process could possibly have.  */
extern int EXFUN(getdtablesize, (NOARGS));


/* Make all changes done to FD actually appear on disk.  */
extern int EXFUN(fsync, (int __fd));

/* Make all changes done to all files actually appear on disk.  */
extern int EXFUN(sync, (NOARGS));


/* Revoke access permissions to all processes currently communicating
   with the control terminal, and then send a SIGHUP signal to the process
   group of the control terminal.  */
extern int EXFUN(vhangup, (NOARGS));


/* Turn accounting on if NAME is an existing file.  The system will then write
   a record for each process as it terminates, to this file.  If NAME is NULL,
   turn accounting off.  This call is restricted to the super-user.  */
extern int EXFUN(acct, (CONST char *__name));

/* Make PATH be the root directory (the starting point for absolute paths).
   This call is restricted to the super-user.  */
extern int EXFUN(chroot, (CONST char *__path));

/* Make the block special device PATH available to the system for swapping.
   This call is restricted to the super-user.  */
extern int EXFUN(swapon, (CONST char *__path));
#endif	/* Use BSD.  */


#endif	/* unistd.h  */
