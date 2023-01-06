/* FILE * interface to a struct __printf_buffer.  Multibyte version.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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

#include <printf_buffer_as_file.h>

#include <assert.h>
#include <printf_buffer.h>

/* Commit the data directly written through the stdio stream.  */
static void
__printf_buffer_as_file_commit (struct __printf_buffer_as_file *file)
{
  /* Check that the write pointers in the file stream are consistent
     with the next buffer.  */
  assert (file->stream._IO_write_ptr >= file->next->write_ptr);
  assert (file->stream._IO_write_ptr <= file->next->write_end);
  assert (file->stream._IO_write_base == file->next->write_base);
  assert (file->stream._IO_write_end == file->next->write_end);

  file->next->write_ptr = file->stream._IO_write_ptr;
}

/* Pointer the FILE * write buffer into the active printf_buffer
   area.  */
static void
__printf_buffer_as_file_switch_to_buffer (struct __printf_buffer_as_file *file)
{
  file->stream._IO_write_base = file->next->write_base;
  file->stream._IO_write_ptr = file->next->write_ptr;
  file->stream._IO_write_end = file->next->write_end;
}

/* Only a small subset of the vtable functions is implemented here,
   following _IO_obstack_jumps.  */

static int
__printf_buffer_as_file_overflow (FILE *fp, int ch)
{
  struct __printf_buffer_as_file *file = (struct __printf_buffer_as_file *) fp;

  __printf_buffer_as_file_commit (file);

  /* EOF means only a flush is requested.   */
  if (ch != EOF)
    __printf_buffer_putc (file->next, ch);

  /* Ensure that flushing actually produces room.  */
  if (!__printf_buffer_has_failed (file->next)
      && file->next->write_ptr == file->next->write_end)
    __printf_buffer_flush (file->next);

  __printf_buffer_as_file_switch_to_buffer (file);

  if (!__printf_buffer_has_failed (file->next))
    return (unsigned char) ch;
  else
    return EOF;
}

static size_t
__printf_buffer_as_file_xsputn (FILE *fp, const void *buf, size_t len)
{
  struct __printf_buffer_as_file *file = (struct __printf_buffer_as_file *) fp;

  __printf_buffer_as_file_commit (file);

  /* Copy the data.  */
  __printf_buffer_write (file->next, buf, len);

  __printf_buffer_as_file_switch_to_buffer (file);

  if (!__printf_buffer_has_failed (file->next))
    return len;
  else
    /* We may actually have written something.  But the stream is
       corrupted in this case anyway, so try not to divine the write
       count here.  */
    return 0;
}

static const struct _IO_jump_t _IO_printf_buffer_as_file_jumps libio_vtable =
{
  JUMP_INIT_DUMMY,
  JUMP_INIT(finish, NULL),
  JUMP_INIT(overflow, __printf_buffer_as_file_overflow),
  JUMP_INIT(underflow, NULL),
  JUMP_INIT(uflow, NULL),
  JUMP_INIT(pbackfail, NULL),
  JUMP_INIT(xsputn, __printf_buffer_as_file_xsputn),
  JUMP_INIT(xsgetn, NULL),
  JUMP_INIT(seekoff, NULL),
  JUMP_INIT(seekpos, NULL),
  JUMP_INIT(setbuf, NULL),
  JUMP_INIT(sync, NULL),
  JUMP_INIT(doallocate, NULL),
  JUMP_INIT(read, NULL),
  JUMP_INIT(write, NULL),
  JUMP_INIT(seek, NULL),
  JUMP_INIT(close, NULL),
  JUMP_INIT(stat, NULL),
  JUMP_INIT(showmanyc, NULL),
  JUMP_INIT(imbue, NULL)
};

void
__printf_buffer_as_file_init (struct __printf_buffer_as_file *file,
                              struct __printf_buffer *next)
{
  file->stream._lock = NULL;
  _IO_no_init (&file->stream, _IO_USER_LOCK, -1, NULL, NULL);
  file->vtable = &_IO_printf_buffer_as_file_jumps;

  /* Set up the write buffer from the next buffer.  */
  file->next = next;
  __printf_buffer_as_file_switch_to_buffer (file);

  /* Mark the read area as inactive, by making all pointers equal.  */
  file->stream._IO_read_base = file->stream._IO_write_base;
  file->stream._IO_read_ptr = file->stream._IO_write_base;
  file->stream._IO_read_end = file->stream._IO_write_base;
}

bool
__printf_buffer_as_file_terminate (struct __printf_buffer_as_file *file)
{
  if (file->stream._flags & _IO_ERR_SEEN)
    return false;
  else
    {
      __printf_buffer_as_file_commit (file);
      return true;
    }
}
