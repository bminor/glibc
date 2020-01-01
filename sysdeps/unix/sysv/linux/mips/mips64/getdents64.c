/* Get directory entries.  Linux/MIPSn64 LFS version.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/param.h>
#include <unistd.h>
#include <limits.h>

#include <include/libc-pointer-arith.h>

ssize_t
__getdents64 (int fd, void *buf, size_t nbytes)
{
  /* The system call takes an unsigned int argument, and some length
     checks in the kernel use an int type.  */
  if (nbytes > INT_MAX)
    nbytes = INT_MAX;

#ifdef __NR_getdents64
  static int getdents64_supported = true;
  if (atomic_load_relaxed (&getdents64_supported))
    {
      ssize_t ret = INLINE_SYSCALL_CALL (getdents64, fd, buf, nbytes);
      if (ret >= 0 || errno != ENOSYS)
	return ret;

      atomic_store_relaxed (&getdents64_supported, false);
    }
#endif

  /* Unfortunately getdents64 was only wire-up for MIPS n64 on Linux 3.10.
     If the syscall is not available it need to fallback to the non-LFS one.
     Also to avoid an unbounded allocation through VLA/alloca or malloc (which
     would make the syscall non async-signal-safe) it uses a limited buffer.
     This is sub-optimal for large NBYTES, however this is a fallback
     mechanism to emulate a syscall that kernel should provide.   */

  struct kernel_dirent
  {
#if _MIPS_SIM == _ABI64
    uint64_t d_ino;
    uint64_t d_off;
#else
    uint32_t d_ino;
    uint32_t d_off;
#endif
    unsigned short int d_reclen;
    char d_name[1];
  };

  /* The largest possible practical length of the d_name member are 255
     Unicode characters in UTF-8 encoding, so d_name is 766 bytes long, plus
     18 (mips64) / 10 (mips64n32) bytes from header, for total of 784 (mips64)
     / 776 (mips64n32) bytes total.  Ensure that the minimum size holds at
     least one entry.  */
  enum { KBUF_SIZE = 1024 };
  char kbuf[KBUF_SIZE];
  size_t kbuf_size = nbytes < KBUF_SIZE ? nbytes : KBUF_SIZE;

  const size_t size_diff = (offsetof (struct dirent64, d_name)
			   - offsetof (struct kernel_dirent, d_name));

  struct dirent64 *dp = (struct dirent64 *) buf;

  size_t nb = 0;
  off64_t last_offset = -1;

  ssize_t r = INLINE_SYSCALL_CALL (getdents, fd, kbuf, kbuf_size);
  if (r <= 0)
    return r;

  struct kernel_dirent *skdp, *kdp;
  skdp = kdp = (struct kernel_dirent *) kbuf;

  while ((char *) kdp < (char *) skdp + r)
    {
      /* This macro is used to avoid aliasing violation.  */
#define KDP_MEMBER(src, member)			     			\
    (__typeof__((struct kernel_dirent){0}.member) *)			\
      memcpy (&((__typeof__((struct kernel_dirent){0}.member)){0}),	\
	      ((char *)(src) + offsetof (struct kernel_dirent, member)),\
	      sizeof ((struct kernel_dirent){0}.member))

      /* This is a conservative approximation, since some of size_diff might
	 fit into the existing padding for alignment.  */
      unsigned short int k_reclen = *KDP_MEMBER (kdp, d_reclen);
      unsigned short int new_reclen = ALIGN_UP (k_reclen + size_diff,
						_Alignof (struct dirent64));
      if (nb + new_reclen > nbytes)
	{
	  /* Entry is too large for the fixed-size buffer.  */
	  if (last_offset == -1)
	    {
	      __set_errno (EINVAL);
	      return -1;
	    }

	  /* The new entry will overflow the input buffer, rewind to last
	     obtained entry and return.  */
	  __lseek64 (fd, last_offset, SEEK_SET);
	  return (char *) dp - (char *) buf;
	}
      nb += new_reclen;

      memcpy (((char *) dp + offsetof (struct dirent64, d_ino)),
	      KDP_MEMBER (kdp, d_ino), sizeof ((struct dirent64){0}.d_ino));
      memcpy (((char *) dp + offsetof (struct dirent64, d_off)),
	      KDP_MEMBER (kdp, d_off), sizeof ((struct dirent64){0}.d_off));
      last_offset = *KDP_MEMBER (kdp, d_off);
      memcpy (((char *) dp + offsetof (struct dirent64, d_reclen)),
	      &new_reclen, sizeof (new_reclen));
      dp->d_type = *((char *) kdp + k_reclen - 1);
      memcpy (dp->d_name, kdp->d_name,
	      k_reclen - offsetof (struct kernel_dirent, d_name));

      dp = (struct dirent64 *) ((char *) dp + new_reclen);
      kdp = (struct kernel_dirent *) (((char *) kdp) + k_reclen);
    }

  return (char *) dp - (char *) buf;
}
libc_hidden_def (__getdents64)
weak_alias (__getdents64, getdents64)

#if _DIRENT_MATCHES_DIRENT64
strong_alias (__getdents64, __getdents)
#endif
