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

/*
 *	ANSI Standard: 4.9 INPUT/OUTPUT	<stdio.h>
 */

#ifndef	_STDIO_H

#if	!defined(__need_FILE)
#define	_STDIO_H	1
#include <features.h>

#define	__need_size_t
#define	__need_NULL
#include <stddef.h>

#define	__need___va_list
#include <stdarg.h>

#define	__need___off_t
#include <sys/types.h>
#endif	/* Don't need FILE.  */
#undef	__need_FILE


#ifndef	__FILE_defined

/* The opaque type of streams.  */
typedef struct __stdio_file FILE;

#define	__FILE_defined	1
#endif	/* FILE not defined.  */


#ifdef	_STDIO_H

/* The type of the second argument to `fgetpos' and `fsetpos'.  */
typedef __off_t fpos_t;

/* The current direction of I/O: reading, writing, or in between.  */
typedef enum { __READING, __WRITING, __CHANGING } __io_direction;


/* The mode of I/O, as given in the MODE argument to fopen, etc.  */
typedef struct
  {
    unsigned int __read:1;	/* Open for reading.  */
    unsigned int __write:1;	/* Open for writing.  */
    unsigned int __append:1;	/* Open for appending.  */
    unsigned int __bin:1;	/* Opened binary.  */
    unsigned int __create:1;	/* Create the file.  */
    unsigned int __exclusive:1;	/* Error if it already exists.  */
    unsigned int __truncate:1;	/* Truncate the file on opening.  */
  } __io_mode;


/* Functions to do I/O and file management for a stream.  */

typedef int EXFUN(__io_read, (PTR __cookie, char *__buf, size_t __nbytes));
typedef int EXFUN(__io_write, (PTR __cookie, CONST char *__buf, size_t __n));
typedef int EXFUN(__io_seek, (PTR __cookie, fpos_t *__pos, int __w));
typedef int EXFUN(__io_close, (PTR __cookie));

/* Low level interface, independent of FILE representation.  */
typedef struct
  {
    __io_read *__read;		/* Read bytes.  */
    __io_write *__write;	/* Write bytes.  */
    __io_seek *__seek;		/* Seek/tell file position.  */
    __io_close *__close;	/* Close file.  */
  } __io_functions;

/* Higher level interface, dependent on FILE representation.  */
typedef struct
  {
    /* Make room in the input buffer.  */
    int EXFUN((*__input), (FILE *__stream));
    /* Make room in the output buffer.  */
    void EXFUN((*__output), (FILE *__stream, int __flush_only, int __c));
  } __room_functions;

extern CONST __io_functions __default_io_functions;
extern CONST __room_functions __default_room_functions;


/* Default close function.  */
extern __io_close __stdio_close;
/* Open FILE with mode M, return cookie or NULL to use an int in *DP.  */
extern PTR EXFUN(__stdio_open, (CONST char *__file, __io_mode __m, int *__dp));
/* Decide the type of buffering and buffer size used for COOKIE.  */
extern void EXFUN(__stdio_linebf, (PTR __cookie, int *__buffering,
				   size_t *__bufsize, __io_functions *__iof,
				   __room_functions *__rf));
/* Put out an error message for when stdio needs to die.  */
extern void EXFUN(__stdio_errmsg, (CONST char *__msg, size_t __len));
/* Generate a unique file name.  */
extern char *EXFUN(__stdio_gen_tempname, (CONST char *__dir, CONST char *__pfx,
					  int __dir_search, size_t *__lenptr));

#ifndef	__NORETURN
#ifdef	__GNUC__
#define	__NORETURN	__volatile
#else	/* Not GCC.  */
#define	__NORETURN
#endif	/* GCC.  */
#endif	/* __NORETURN not defined.  */

/* Print out MESSAGE on the error output and abort.  */
extern __NORETURN void EXFUN(__libc_fatal, (CONST char *__message));


/* The FILE structure.  */
struct __stdio_file
  {
    FILE *__next;			/* Next FILE in the linked list.  */
    char *__buf;			/* Base of buffer.  */
    char *__bp;				/* Pointer into the buffer.  */
    char *__end;			/* Pointer past end of the buffer.  */
    size_t __bufsz;			/* Size of the buffer.  */
    PTR __crunch;			/* Magic cookie.  */
    int __fd;				/* System file descriptor.  */
    unsigned char __magic;		/* Magic number for validation.  */
#define	_IOMAGIC 0xd5			/* Magic number to fill `__magic'.  */
    char __pb_buf[3];			/* Pushback buffer.  */
    size_t __pb_count;			/* Count of chars pushed back.  */
    __io_mode __iomode;			/* File access mode.  */
    unsigned int __eof:1;		/* End of file encountered.  */
    unsigned int __error:1;		/* Error encountered.  */
    unsigned int __usrbuf:1;		/* Buffer is from user.  */
    unsigned int __lnbuf:1;		/* Flush on newline.  */
    unsigned int __beenseen:1;		/* This stream has been seen.  */
    unsigned int __isapipe:1;		/* Nonzero if opened by popen.  */
    unsigned int __iodir:2;		/* Current direction of I/O.  */
    __io_functions __iofuncs;		/* I/O functions.  */
    __room_functions __roomfuncs;	/* I/O buffer room functions.  */
    fpos_t __off;			/* Offset of the buffer.  */
  };


/* All macros used internally by other macros here and by stdio functions begin
   with `__'.  All of these may evaluate their arguments more than once.  */


/* Nonzero if STREAM is a valid stream.  */
#define	__validfp(stream) ((stream) != NULL && (stream)->__magic == _IOMAGIC)


/* Check to see if I/O for STREAM is going in
   direction DIR and signal a fatal error if so.  */
#define	__ill_dirch_check(stream, dir)	\
  (__direction(stream) == (dir) && (__stdio_ill_dirch(dir), 0))

/* Print a message about an illegal direction change and abort.  */
extern __NORETURN void EXFUN(__stdio_ill_dirch, (__io_direction __dir));


/* This must be called before each function that does output.  */
#define	__beginwrite(stream)						      \
  do									      \
    switch (__direction(stream))					      \
      {									      \
      case __READING:							      \
	__stdio_ill_dirch(__READING);					      \
	break;								      \
      case __WRITING:							      \
	break;								      \
      case __CHANGING:							      \
	/* The buffer is empty.  */					      \
        __bufp(stream) = __buffer(stream);				      \
        if (__linebuf(stream))						      \
	  __endp(stream) = __bufp(stream);				      \
	__set_direction(stream, __WRITING);				      \
      }									      \
  while (0)


/* These macros hide the member names of the FILE structure by providing
   access to their information in the form of pseudo-functions.  */

#define	__direction(stream)	((__io_direction) (stream)->__iodir)
#define	__set_direction(stream, direction)	\
  ((stream)->__iodir = (unsigned int) (direction))
#define	__seen(stream)		((stream)->__beenseen)
#define	__linebuf(stream)	((stream)->__lnbuf)
#define	__userbuf(stream)	((stream)->__usrbuf)
#define	__ispipe(stream)	((stream)->__isapipe)

#define	__mode(stream)		((stream)->__iomode)
#define	__canread(stream)	(__mode(stream).__read)
#define	__canwrite(stream)	(__mode(stream).__write)
#define	__binary(stream)	(__mode(stream).__bin)

#define	__buffer(stream)	((stream)->__buf)
#define	__bufp(stream)		((stream)->__bp)
#define	__bufsize(stream)	((stream)->__bufsz)
#define	__endp(stream)		((stream)->__end)

#define	__charsleft(stream)	(__endp(stream) - __bufp(stream))

#define	__feof(stream)		((stream)->__eof)
#define	__ferror(stream)	((stream)->__error)

#define	__cookie(stream)	((stream)->__crunch)
#define	__fileno(stream)	((stream)->__fd)

#define	__io_funcs(stream)	((stream)->__iofuncs)
#define	__room_funcs(stream)	((stream)->__roomfuncs)

#define	__offset(stream)	((stream)->__off)


/* Clear the error and EOF indicators of STREAM.  */
#define	__clearerr(stream)	(__ferror(stream) = __feof(stream) = 0)


/* Set the magic cookie to the fd and the I/O functions to their
   defaults and set the `seen' flag so it won't be done again.  */
#define	__see(stream)							      \
  ((void) (__cookie(stream) = (PTR) &__fileno(stream),			      \
   __io_funcs(stream) = __default_io_functions,				      \
   __room_funcs(stream) = __default_room_functions,			      \
   __seen(stream) = 1))


/* Return one of the standard buffering-method constants,
   telling which method is used by STREAM.  */
#define	__buffering(stream)						      \
  ((__buffer(stream) && __buffer(stream) != (stream)->__pb_buf) ?	      \
   (__linebuf(stream) ? _IOLBF : _IOFBF) :				      \
   (__userbuf(stream) ? _IONBF : _IOFBF))				      \


/* Nuke STREAM, making it unusable but available for reuse.  */
extern void EXFUN(__invalidate, (FILE *__stream));


/* The possibilities for the third argument to `setvbuf'.  */
#define _IOFBF	0x1	/* Full buffering.  */
#define _IOLBF	0x2	/* Line buffering.  */
#define _IONBF	0x4	/* No buffering.  */


/* Default buffer size.  */
#define	BUFSIZ	1024


/* End of file character.
   Some things throughout the library rely on this being -1.  */
#define	EOF	(-1)


/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define	SEEK_SET	0	/* Seek from beginning of file.  */
#define	SEEK_CUR	1	/* Seek from current position.  */
#define	SEEK_END	2	/* Seek from end of file.  */


#ifdef	__USE_SVID
/* Default path prefix for `tempnam' and `tmpnam'.  */
#define	P_tmpdir	"/usr/tmp"
#endif


/* Get the values:
   L_tmpnam	How long an array of chars must be to be passed to `tmpnam'.
   TMP_MAX	The minimum number of unique filenames generated by tmpnam
   		(and tempnam when it uses tmpnam's name space),
		or tempnam (the two are separate).
   L_ctermid	How long an array to pass to `ctermid'.
   L_cuserid	How long an array to pass to `cuserid'.
   FOPEN_MAX	Mininum number of files that can be open at once.
   FILENAME_MAX	Maximum length of a filename.  */
#include <stdio_limits.h>


/* All the known streams are in a linked list
   linked by the `next' field of the FILE structure.  */

extern FILE *__stdio_head;	/* Head of the list.  */
extern FILE *__stdio_tail;	/* Tail of the list.  */

/* The standard streams (standard input, output, and error) must be
   defined statically because they are never opened.  They are still
   part of the linked list; _stdio_head is initialized to point to
   the first of them and _stdio_tail to the last and all of their
   `next' fields point to the appropriate next address.  */
extern FILE stdin[1], stdout[1], stderr[1];


/* Remove file FILENAME.  */
extern int EXFUN(remove, (CONST char *__filename));
/* Rename file OLD to NEW.  */
extern int EXFUN(rename, (CONST char *__old, CONST char *__new));


/* Create a temporary file and open it read/write.  */
extern FILE *EXFUN(tmpfile, (NOARGS));
/* Generate a temporary filename.  */
extern char *EXFUN(tmpnam, (char *__s));


#ifdef	__USE_SVID
/* Generate a unique temporary filename using up to five characters of PFX
   if it is not NULL.  The directory to put this file in is searched for
   as follows: First the environment variable "TMPDIR" is checked.
   If it contains the name of a writable directory, that directory is used.
   If not and if DIR is not NULL, that value is checked.  If that fails,
   P_tmpdir is tried and finally "/tmp".  The storage for the filename
   is allocated by `malloc'.  */
extern char *EXFUN(tempnam, (CONST char *__dir, CONST char *__pfx));
#endif


/* This performs actual output when necessary, flushing
   STREAM's buffer and optionally writing another character.  */
extern int EXFUN(__flshfp, (FILE *__stream, int __c,
			    int __flush_only));


/* Close STREAM, or all streams if STREAM is NULL.  */
extern int EXFUN(fclose, (FILE *__stream));
/* Flush STREAM, or all streams if STREAM is NULL.  */
extern int EXFUN(fflush, (FILE *__stream));


/* Open a file and create a new stream for it.  */
extern FILE *EXFUN(fopen, (CONST char *__filename, CONST char *__modes));
/* Open a file, replacing an existing stream with it. */
extern FILE *EXFUN(freopen, (CONST char *__filename,
			     CONST char *__modes, FILE *__stream));

/* Create a new stream that refers to the given magic cookie,
   and uses the given functions for input and output. (Used internally.)  */
extern FILE *EXFUN(__fopencookie, (PTR __magic_cookie, __io_mode mode,
				   __io_functions __io_functions));

#ifdef	__USE_POSIX
/* Create a new stream that refers to an existing system file descriptor.  */
extern FILE *EXFUN(fdopen, (int __fd, CONST char *__modes));
#endif

#ifdef	__USE_GNU
/* Create a new stream that refers to the given magic cookie,
   and uses the given functions for input and output.  */
extern FILE *EXFUN(fopencookie, (PTR __magic_cookie, CONST char *__modes,
				 __io_functions __io_functions));

/* Create a new stream that refers to a memory buffer.  */
extern FILE *EXFUN(fmemopen, (char *__s, size_t __len, CONST char *__modes));
#endif


/* If BUF is NULL, make STREAM unbuffered.
   Else make it use buffer BUF, of size BUFSIZ.  */
extern void EXFUN(setbuf, (FILE *__stream, char *__buf));
/* Make STREAM use buffering mode MODE.
   If BUF is not NULL, use N bytes of it for buffering;
   else allocate an internal buffer N bytes long.  */
extern int EXFUN(setvbuf, (FILE *__stream, char *__buf,
			   int __modes, size_t __n));

#ifdef	__USE_BSD
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use SIZE bytes of BUF for buffering.  */
extern void	EXFUN(setbuffer, (FILE *__stream, char *__buf, size_t __size));

/* Make STREAM line-buffered.  */
extern void EXFUN(setlinebuf, (FILE *__stream));
#endif


/* Write formatted output to STREAM.  */
extern int EXFUN(fprintf, (FILE *__stream, CONST char *__format, ...));
/* Write formatted output to stdout.  */
extern int EXFUN(printf, (CONST char *__format, ...));
/* Write formatted output to S.  */
extern int EXFUN(sprintf, (char *__s, CONST char *__format, ...));

/* Write formatted output to S from argument list ARG.  */
extern int EXFUN(vfprintf, (FILE *__s, CONST char *__format, __va_list __arg));
/* Write formatted output to stdout from argument list ARG.  */
extern int EXFUN(vprintf, (CONST char *__format, __va_list __arg));
/* Write formatted output to S from argument list ARG.  */
extern int EXFUN(vsprintf, (char *__s, CONST char *__format, __va_list __arg));

/* Write formatted output to S from argument list ARG.
   If LIMIT is nonzero, process no more than MAX specs.  */
extern int EXFUN(__vflprintf, (FILE *__s, CONST char *__format,
			       __va_list __arg, int __limit, size_t __max));

#ifdef	__OPTIMIZE__
#define	vfprintf(s, format, arg)	__vflprintf((s), (format), (arg), 0, 0)
#define	vprintf(fmt, arg)		__vflprintf(stdout, (fmt), (arg), 0, 0)
#endif	/* Optimizing.  */

#ifdef	__USE_GNU
/* Maximum chars of output to write in MAXLEN.  */
extern int EXFUN(snprintf, (char *__s, size_t __maxlen,
			    CONST char *__format, ...));

extern int EXFUN(vsnprintf, (char *__s, size_t __maxlen,
			     CONST char *__format, __va_list __arg));

/* Process no more than N format specifiers.  */
extern int EXFUN(vflprintf, (FILE *__s, size_t __n,
			     CONST char *__format, __va_list __arg));

#ifdef	__OPTIMIZE__
#define	vflprintf(s, n, fmt, arg)	__vflprintf((s), (fmt), (arg), 1, (n))
#endif	/* Optimizing.  */

extern int EXFUN(vslprintf, (char *__s, size_t __n,
			     CONST char *__format, __va_list __arg));

/* Write formatted output to S from FORMAT with arguments from ARG,
   writing no more than MAXLEN chars and processing no more than N specs.  */
extern int EXFUN(vslnprintf, (char *__s, size_t __n, size_t __maxlen,
			      CONST char *__format, __va_list __arg));


/* Write formatted output to a string dynamically allocated with `malloc'.
   Store the address of the string in *PTR.  */
extern int EXFUN(vasprintf, (char **__ptr, CONST char *__f, __va_list __arg));
extern int EXFUN(asprintf, (char **__ptr, CONST char *__fmt, ...));
#endif


/* Read formatted input from STREAM.  */
extern int EXFUN(fscanf, (FILE *__stream, CONST char *__format, ...));
/* Read formatted input from stdin.  */
extern int EXFUN(scanf, (CONST char *__format, ...));
/* Read formatted input from S.  */
extern int EXFUN(sscanf, (CONST char *__s, CONST char *__format, ...));

#ifdef	__USE_GNU
/* Read formatted input from S into argument list ARG.  */
extern int EXFUN(__vfscanf, (FILE *__s, CONST char *__format,
			     __va_list __arg));
extern int EXFUN(vfscanf, (FILE *__s, CONST char *__format,
			   __va_list __arg));

/* Read formatted input from stdin into argument list ARG.  */
extern int EXFUN(vscanf, (CONST char *__format, __va_list __arg));

/* Read formatted input from S into argument list ARG.  */
extern int EXFUN(__vsscanf, (CONST char *__s, CONST char *__format,
			     __va_list __arg));
extern int EXFUN(vsscanf, (CONST char *__s, CONST char *__format,
			   __va_list __arg));


#ifdef	__OPTIMIZE__
#define	vfscanf(s, format, arg)	__vfscanf((s), (format), (arg))
#define	vscanf(format, arg)	__vfscanf(stdin, (format), (arg))
#define	vsscanf(s, format, arg)	__vsscanf((s), (format), (arg))
#endif	/* Optimizing.  */
#endif	/* Use GNU.  */


/* This does actual reading when necessary, filling STREAM's
   buffer and returning the first character in it.  */
extern int EXFUN(__fillbf, (FILE *__stream));


/* Read a character from STREAM.  */
extern int EXFUN(fgetc, (FILE *__stream));
extern int EXFUN(getc, (FILE *__stream));

/* Read a character from stdin.  */
extern int EXFUN(getchar, (NOARGS));

/* The C standard explicitly says this can
   re-evaluate its argument, so it does. */
#define	__getc(stream)	\
  (__bufp(stream) == __endp(stream) ? __fillbf(stream) : \
   (int) ((unsigned char) *__bufp(stream)++))

/* The C standard explicitly says this is a macro,
   so we always do the optimization for it.  */
#define	getc(stream)	__getc(stream)

#ifdef	__OPTIMIZE__
#define	getchar()	__getc(stdin)
#endif	/* Optimizing.  */


/* Write a character to STREAM.  */
extern int EXFUN(fputc, (int __c, FILE *__stream));
extern int EXFUN(putc, (int __c, FILE *__stream));

/* Write a character to stdout.  */
extern int EXFUN(putchar, (int __c));

/* The C standard explicitly says this can
   re-evaluate its arguments, so it does.  */
#define	__putc(c, stream)						      \
  (__bufp(stream) == __endp(stream) ?					      \
   __flshfp((stream), (c), 0) : (unsigned char) (*__bufp(stream)++ = (c)))

/* The C standard explicitly says this can be a macro,
   so we always do the optimization for it.  */
#define	putc(c, stream)	__putc((c), (stream))

#ifdef	__OPTIMIZE__
#define	putchar(c)	__putc((c), stdout)
#endif	/* Optimizing.  */


#if defined(__USE_SVID) || defined(__USE_MISC)
/* Get a word (int) from STREAM.  */
extern int EXFUN(getw, (FILE *__stream));

/* Write a word (int) to STREAM.  */
extern int EXFUN(putw, (int __w, FILE *__stream));
#endif


/* Get a newline-terminated string of finite length from STREAM.  */
extern char *EXFUN(fgets, (char *__s, size_t __n, FILE *__stream));

/* Get a newline-terminated string from stdin, removing the newline.
   DO NOT USE THIS FUNCTION!!  There is no limit on how much it will read.  */
extern char *EXFUN(gets, (char *__s));


/* Write a string to STREAM.  */
extern int EXFUN(fputs, (CONST char *__s, FILE *__stream));
/* Write a string, followed by a newline, to stdout.  */
extern int EXFUN(puts, (CONST char *__s));

#ifdef	__OPTIMIZE__
#define	puts(s)	((fputs((s), stdout) || __putc('\n', stdout) == EOF) ? EOF : 0)
#endif	/* Optimizing.  */


/* Push a character back onto the input buffer of STREAM.  */
extern int EXFUN(ungetc, (int __c, FILE *__stream));


/* Read chunks of generic data from STREAM.  */
extern size_t EXFUN(fread, (PTR __ptr, size_t __size,
			    size_t __n, FILE *__stream));
/* Write chunks of generic data to STREAM.  */
extern size_t EXFUN(fwrite, (CONST PTR __ptr, size_t __size,
			     size_t __n, FILE *__s));


/* Seek to a certain position on STREAM.  */
extern int EXFUN(fseek, (FILE *__stream, long int __off, int __whence));
/* Return the current position of STREAM.  */
extern long int EXFUN(ftell, (FILE *__stream));
/* Rewind to the beginning of STREAM.  */
extern void EXFUN(rewind, (FILE *__stream));

/* Get STREAM's position.  */
extern int EXFUN(fgetpos, (FILE *__stream, fpos_t *__pos));
/* Set STREAM's position.  */
extern int EXFUN(fsetpos, (FILE *__stream, CONST fpos_t *__pos));


/* Clear the error and EOF indicators for STREAM.  */
extern void EXFUN(clearerr, (FILE *__stream));
/* Return the EOF indicator for STREAM.  */
extern int EXFUN(feof, (FILE *__stream));
/* Return the error indicator for STREAM.  */
extern int EXFUN(ferror, (FILE *__stream));

#ifdef	__OPTIMIZE__
#define	feof(stream)	(__feof(stream) != 0)
#define	ferror(stream)	(__ferror(stream) != 0)
#endif	/* Optimizing.  */


/* Print a message describing the meaning of the value of errno.  */
extern void EXFUN(perror, (CONST char *__s));

#ifdef	__USE_MISC
/* Print a message describing the meaning of the given signal number.  */
extern void EXFUN(psignal, (int __sig, CONST char *__s));
#endif	/* Non strict ANSI and not POSIX only.  */


#ifdef	__USE_POSIX
/* Return the system file descriptor for STREAM.  */
extern int EXFUN(fileno, (CONST FILE *__stream));

#ifdef	__OPTIMIZE__
/* The `+ 0' makes this not be an lvalue, so it can't be changed.  */
#define	fileno(stream)	(__fileno(stream) + 0)
#endif	/* Optimizing.  */

#endif	/* Use POSIX.  */


#if (defined (__USE_POSIX2) || defined(__USE_SVID) || defined(__USE_BSD) || \
     defined(__USE_MISC))
/* Create a new stream connected to a pipe running the given command.  */
extern FILE *EXFUN(popen, (CONST char *__command, CONST char *__modes));

/* Close a stream opened by popen and return the status of its child.  */
extern int EXFUN(pclose, (FILE *__stream));
#endif


#ifdef	__USE_POSIX
/* Return the name of the controlling terminal.  */
extern char *EXFUN(ctermid, (char *__s));
/* Return the name of the current user.  */
extern char *EXFUN(cuserid, (char *__s));
#endif

#endif	/* <stdio.h> included.  */

#endif	/* stdio.h  */
