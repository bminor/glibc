/* Checking macros for unistd functions.
   Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _UNISTD_H
# error "Never include <bits/unistd.h> directly; use <unistd.h> instead."
#endif

extern void __chk_fail (void) __attribute__((noreturn));
#define read(fd, buf, nbytes) \
  (__extension__							      \
    ({ size_t __nbytes_val = (nbytes);			  		      \
       if (__bos0 (buf) != (size_t) -1 && __bos0 (buf) < __nbytes_val)	      \
         __chk_fail ();							      \
       read (fd, buf, __nbytes_val); }))

#ifdef __USE_UNIX98
# define pread(fd, buf, nbytes, offset) \
  (__extension__							      \
    ({ size_t __nbytes_val = (nbytes);			  		      \
       if (__bos0 (buf) != (size_t) -1 && __bos0 (buf) < __nbytes_val)	      \
         __chk_fail ();							      \
       pread (fd, buf, __nbytes_val, offset); }))

# ifdef __USE_LARGEFILE64
#  define pread64(fd, buf, nbytes, offset) \
  (__extension__							      \
    ({ size_t __nbytes_val = (nbytes);			  		      \
       if (__bos0 (buf) != (size_t) -1 && __bos0 (buf) < __nbytes_val)	      \
         __chk_fail ();							      \
       pread64 (fd, buf, __nbytes_val, offset); }))
# endif
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K
# define readlink(path, buf, len) \
  (__extension__							      \
    ({ size_t __len_val = (len);			  		      \
       if (__bos (buf) != (size_t) -1 && __bos (buf) < __len_val)	      \
         __chk_fail ();							      \
       readlink (path, buf, __len_val); }))
#endif

#define getcwd(buf, size) \
  (__extension__							      \
    ({ size_t __size_val = (size);			  		      \
       if (__bos (buf) != (size_t) -1 && __bos (buf) < __size_val)	      \
         __chk_fail ();							      \
       getcwd (buf, __size_val); }))
