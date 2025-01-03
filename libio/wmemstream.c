/* Copyright (C) 1995-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include "libioP.h"
#include "strfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>


struct _IO_FILE_wmemstream
{
  _IO_strfile _sf;
  wchar_t **bufloc;
  size_t *sizeloc;
};


/* Open a stream that writes into a malloc'd buffer that is expanded as
   necessary.  *BUFLOC and *SIZELOC are updated with the buffer's location
   and the number of characters written on fflush or fclose.  */
FILE *
open_wmemstream (wchar_t **bufloc, size_t *sizeloc)
{
  struct locked_FILE
  {
    struct _IO_FILE_wmemstream fp;
#ifdef _IO_MTSAFE_IO
    _IO_lock_t lock;
#endif
    struct _IO_wide_data wd;
  } *new_f;
  wchar_t *buf;

  new_f = (struct locked_FILE *) malloc (sizeof (struct locked_FILE));
  if (new_f == NULL)
    return NULL;
#ifdef _IO_MTSAFE_IO
  new_f->fp._sf._sbf._f._lock = &new_f->lock;
#endif

  buf = calloc (1, BUFSIZ);
  if (buf == NULL)
    {
      free (new_f);
      return NULL;
    }
  _IO_no_init (&new_f->fp._sf._sbf._f, 0, 0, &new_f->wd, &_IO_wmem_jumps);
  _IO_fwide (&new_f->fp._sf._sbf._f, 1);
  _IO_wstr_init_static (&new_f->fp._sf._sbf._f, buf,
			BUFSIZ / sizeof (wchar_t), buf);
  new_f->fp._sf._sbf._f._flags2 &= ~_IO_FLAGS2_USER_WBUF;
  new_f->fp._sf._s._allocate_buffer_unused = (_IO_alloc_type) malloc;
  new_f->fp._sf._s._free_buffer_unused = (_IO_free_type) free;

  new_f->fp.bufloc = bufloc;
  new_f->fp.sizeloc = sizeloc;

  /* Disable single thread optimization.  BZ 21735.  */
  new_f->fp._sf._sbf._f._flags2 |= _IO_FLAGS2_NEED_LOCK;

  return (FILE *) &new_f->fp._sf._sbf;
}


int
_IO_wmem_sync (FILE *fp)
{
  struct _IO_FILE_wmemstream *mp = (struct _IO_FILE_wmemstream *) fp;

  if (fp->_wide_data->_IO_write_ptr == fp->_wide_data->_IO_write_end)
    {
      _IO_wstr_overflow (fp, '\0');
      --fp->_wide_data->_IO_write_ptr;
    }

  *mp->bufloc = fp->_wide_data->_IO_write_base;
  *mp->sizeloc = (fp->_wide_data->_IO_write_ptr
		  - fp->_wide_data->_IO_write_base);

  return 0;
}


void
_IO_wmem_finish (FILE *fp, int dummy)
{
  struct _IO_FILE_wmemstream *mp = (struct _IO_FILE_wmemstream *) fp;

  *mp->bufloc = (wchar_t *) realloc (fp->_wide_data->_IO_write_base,
				     (fp->_wide_data->_IO_write_ptr
				      - fp->_wide_data->_IO_write_base + 1)
				     * sizeof (wchar_t));
  if (*mp->bufloc != NULL)
    {
      size_t len = (fp->_wide_data->_IO_write_ptr
		    - fp->_wide_data->_IO_write_base);
      (*mp->bufloc)[len] = '\0';
      *mp->sizeloc = len;

      fp->_wide_data->_IO_buf_base = NULL;
    }

  _IO_wstr_finish (fp, 0);
}
