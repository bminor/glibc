/* This file is generated from errno.texinfo by errnos.awk. */

#ifdef _ERRNO_H

/* The Hurd uses Mach error system 0x10, currently only subsystem 0.  */
#define _HURD_ERRNO(n)	((0x10 << 26) | ((n) & 0x3fff))
#define	EPERM           _HURD_ERRNO (1)	/* Operation not permitted */
#define	ENOENT          _HURD_ERRNO (2)	/* No such file or directory */
#define	ESRCH           _HURD_ERRNO (3)	/* No such process */
#define	EINTR           _HURD_ERRNO (4)	/* Interrupted system call */
#define	EIO             _HURD_ERRNO (5)	/* Input/output error */
#define	ENXIO           _HURD_ERRNO (6)	/* Device not configured */
#define	E2BIG           _HURD_ERRNO (7)	/* Argument list too long */
#define	ENOEXEC         _HURD_ERRNO (8)	/* Exec format error */
#define	EBADF           _HURD_ERRNO (9)	/* Bad file descriptor */
#define	ECHILD          _HURD_ERRNO (10)	/* No child processes */
#define	EDEADLK         _HURD_ERRNO (11)	/* Resource deadlock avoided */
#define	ENOMEM          _HURD_ERRNO (12)	/* Cannot allocate memory */
#define	EACCES          _HURD_ERRNO (13)	/* Permission denied */
#define	EFAULT          _HURD_ERRNO (14)	/* Bad address */
#define	ENOTBLK         _HURD_ERRNO (15)	/* Block device required */
#define	EBUSY           _HURD_ERRNO (16)	/* Device busy */
#define	EEXIST          _HURD_ERRNO (17)	/* File exists */
#define	EXDEV           _HURD_ERRNO (18)	/* Invalid cross-device link */
#define	ENODEV          _HURD_ERRNO (19)	/* Operation not supported by device */
#define	ENOTDIR         _HURD_ERRNO (20)	/* Not a directory */
#define	EISDIR          _HURD_ERRNO (21)	/* Is a directory */
#define	EINVAL          _HURD_ERRNO (22)	/* Invalid argument */
#define	EMFILE          _HURD_ERRNO (23)	/* Too many open files */
#define	ENFILE          _HURD_ERRNO (24)	/* Too many open files in system */
#define	ENOTTY          _HURD_ERRNO (25)	/* Inappropriate ioctl for device */
#define	ETXTBSY         _HURD_ERRNO (26)	/* Text file busy */
#define	EFBIG           _HURD_ERRNO (27)	/* File too large */
#define	ENOSPC          _HURD_ERRNO (28)	/* No space left on device */
#define	ESPIPE          _HURD_ERRNO (29)	/* Illegal seek */
#define	EROFS           _HURD_ERRNO (30)	/* Read-only file system */
#define	EMLINK          _HURD_ERRNO (31)	/* Too many links */
#define	EPIPE           _HURD_ERRNO (32)	/* Broken pipe */
#endif /* <errno.h> included.  */
#if (!defined (__Emath_defined) && (defined (_ERRNO_H) || defined (__need_Emath)))
#define	EDOM            _HURD_ERRNO (33)	/* Numerical argument out of domain */
#endif /* Emath not defined and <errno.h> included or need Emath.  */
#ifdef _ERRNO_H
#endif /* <errno.h> included.  */
#if (!defined (__Emath_defined) && (defined (_ERRNO_H) || defined (__need_Emath)))
#define	ERANGE          _HURD_ERRNO (34)	/* Numerical result out of range */
#endif /* Emath not defined and <errno.h> included or need Emath.  */
#ifdef _ERRNO_H
#define	EAGAIN          _HURD_ERRNO (35)	/* Resource temporarily unavailable */
#define	EWOULDBLOCK     _HURD_ERRNO (36)	/* Operation would block */
#define	EINPROGRESS     _HURD_ERRNO (37)	/* Operation now in progress */
#define	EALREADY        _HURD_ERRNO (38)	/* Operation already in progress */
#define	ENOTSOCK        _HURD_ERRNO (39)	/* Socket operation on non-socket */
#define	EDESTADDRREQ    _HURD_ERRNO (40)	/* Destination address required */
#define	EMSGSIZE        _HURD_ERRNO (41)	/* Message too long */
#define	EPROTOTYPE      _HURD_ERRNO (42)	/* Protocol wrong type for socket */
#define	ENOPROTOOPT     _HURD_ERRNO (43)	/* Protocol not available */
#define	EPROTONOSUPPORT _HURD_ERRNO (44)	/* Protocol not supported */
#define	ESOCKTNOSUPPORT _HURD_ERRNO (45)	/* Socket type not supported */
#define	EOPNOTSUPP      _HURD_ERRNO (46)	/* Operation not supported */
#define	EPFNOSUPPORT    _HURD_ERRNO (47)	/* Protocol family not supported */
#define	EAFNOSUPPORT    _HURD_ERRNO (48)	/* Address family not supported by protocol family */
#define	EADDRINUSE      _HURD_ERRNO (49)	/* Address already in use */
#define	EADDRNOTAVAIL   _HURD_ERRNO (50)	/* Can't assign requested address */
#define	ENETDOWN        _HURD_ERRNO (51)	/* Network is down */
#define	ENETUNREACH     _HURD_ERRNO (52)	/* Network is unreachable */
#define	ENETRESET       _HURD_ERRNO (53)	/* Network dropped connection on reset */
#define	ECONNABORTED    _HURD_ERRNO (54)	/* Software caused connection abort */
#define	ECONNRESET      _HURD_ERRNO (55)	/* Connection reset by peer */
#define	ENOBUFS         _HURD_ERRNO (56)	/* No buffer space available */
#define	EISCONN         _HURD_ERRNO (57)	/* Socket is already connected */
#define	ENOTCONN        _HURD_ERRNO (58)	/* Socket is not connected */
#define	ESHUTDOWN       _HURD_ERRNO (59)	/* Can't send after socket shutdown */
#define	ETIMEDOUT       _HURD_ERRNO (60)	/* Connection timed out */
#define	ECONNREFUSED    _HURD_ERRNO (61)	/* Connection refused */
#define	ELOOP           _HURD_ERRNO (62)	/* Too many levels of symbolic links */
#define	ENAMETOOLONG    _HURD_ERRNO (63)	/* File name too long */
#define	EHOSTDOWN       _HURD_ERRNO (64)	/* Host is down */
#define	EHOSTUNREACH    _HURD_ERRNO (65)	/* No route to host */
#define	ENOTEMPTY       _HURD_ERRNO (66)	/* Directory not empty */
#define	EUSERS          _HURD_ERRNO (67)	/* Too many users */
#define	EDQUOT          _HURD_ERRNO (68)	/* Disc quota exceeded */
#define	ESTALE          _HURD_ERRNO (69)	/* Stale NFS file handle */
#define	EREMOTE         _HURD_ERRNO (70)	/* Too many levels of remote in path */
#define	ENOLCK          _HURD_ERRNO (71)	/* No locks available */
#define	ENOSYS          _HURD_ERRNO (72)	/* Function not implemented */
#define	EBACKGROUND     _HURD_ERRNO (73)	/* Inappropriate operation for background process */
#define	ED              _HURD_ERRNO (74)	/* ? */
#define	EGREGIOUS       _HURD_ERRNO (75)	/* You really blew it this time */
#define	EIEIO           _HURD_ERRNO (76)	/* Computer bought the farm */
#define	EGRATUITOUS     _HURD_ERRNO (77)	/* Gratuitous error */
#define _HURD_ERRNOS 78
#endif /* <errno.h> included.  */
