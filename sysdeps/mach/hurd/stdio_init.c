#include <sysdeps/stub/stdio_init.c>
#if 0

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

#include <ansidecl.h>
#include <stdio.h>
#include <hurd.h>
#include <hurd/shared.h>

struct mapped
  {
    struct shared_io shared;

    /* The rest of the shared page is owned by the user (stdio).  */

    io_t file;			/* Port to the file.  */
    size_t blksize;		/* Optimal size for i/o.  */
    memory_object_t cntl;	/* Memory object for this page.  */
    /* Memory objects for reading and writing data.
       These two might be the same.  */
    memory_object_t rdmemobj, wrmemobj;
  };


/* Get IT.  */
#ifdef	__GNUC__
__inline
#endif
static error_t
DEFUN(get_it, (m), struct mapped *m)
{
 try:

  __spin_lock(m->shared.lock);

  switch (m->shared.it_status)
    {
    case USER_POTENTIALLY_IT:
      m->shared.it_status = USER_IT;
    case USER_RELEASE_IT:
    case USER_IT:
      __spin_unlock(m->shared.lock);
      return 0;

    case USER_NOT_IT:
      __spin_unlock(m->shared.lock);
      {
	error_t error = __io_get_it(m->file, m->cntl);
	if (error == 0)
	  return 0;
	else
	  /* Don't just tail-recurse because that might
	     make the function not get inlined.  Sigh.  */
	  goto try;
      }

    default:
      __libc_fatal("get_it: Unrecognized IT status!\n");
    }
}

/* Release IT.  */
#ifdef	__GNUC__
__inline
#endif
static error_t
DEFUN(release_it, (m), struct mapped *m)
{
  __spin_lock(m->shared.lock);
  switch (m->shared.it_status)
    {
    case USER_IT:
      m->shared.it_status = USER_POTENTIALLY_IT;
    case USER_NOT_IT:
      __spin_unlock(m->shared.lock);
      return 0;

    case USER_RELEASE_IT:
      __spin_unlock(m->shared.lock);
      return __io_release_it(m->file, m->cntl);

    default:
      __libc_fatal("release_it: Unrecognized IT status!\n");
    }
}

static int
DEFUN(mapped_close, (cookie), PTR cookie)
{
  struct mapped *CONST m = (struct mapped *) cookie;
  int am_it;
  error_t error = 0;

  __spin_lock(m->shared.lock);
  am_it = m->shared.it_status != USER_NOT_IT;
  __spin_unlock(m->shared.lock);
  if (am_it)
    error = __io_release_it(m->file, m->cntl);

#define	DO(foo)	if (error == 0) error = foo; else (void) foo
  DO(__vm_deallocate(__mach_task_self(), m, sizeof(*m)));
  DO(__port_deallocate(__mach_task_self(), m->file));
  DO(__port_deallocate(__mach_task_self(), m->cntl));
  DO(__port_deallocate(__mach_task_self(), m->rdmemobj));
  DO(__port_deallocate(__mach_task_self(), m->wrmemobj));
#undef	DO

  if (error != 0)
    {
      errno = error;
      return -1;
    }
  return 0;
}

static int
DEFUN(mapped_seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  struct shared_io *CONST m = (struct shared_io *) cookie;
  int error;

  error = get_it(m);
  if (error == 0)
    {
      switch (whence)
	{
	case SEEK_SET:
	  if (!m->seekable && *pos > m->file_pointer)
	    error = ESPIPE;
	  else
	    {
	      m->file_pointer = *pos;
	      error = 0;
	    }
	  break;

	case SEEK_CUR:
	  if (!m->seekable && *pos < 0)
	    error = ESPIPE;
	  else
	    {
	      m->file_pointer += *pos;
	      *pos = m->file_pointer;
	      error = 0;
	    }
	  break;

	case SEEK_END:
	  if (!m->use_file_size)
	    error = ESPIPE;
	  else
	    {
	      off_t desired = m->file_size + *pos;
	      if (!m->seekable && desired < m->file_pointer)
		error = ESPIPE;
	      else
		{
		  *pos = m->file_pointer = desired;
		  error = 0;
		}
	    }
	  break;

	default:
	  error = EINVAL;
	  break;
	}

      if (error == 0)
	error = release_it(m);
      else
	(void) release_it(m);
    }

  if (error != 0)
    {
      errno = error;
      return -1;
    }
  return 0;
}

static int
DEFUN(mapped_input, (stream), FILE *stream)
{
  struct mapped *CONST m = (struct mapped *) cookie;
  struct shared_io *CONST sio = &m->shared;
  off_t pos, bufp;
  error_t error;
  off_t size;

  if (error = get_it(sio))
    {
      __error(stream) = 1;
      errno = error;
      return EOF;
    }

  if (__buffer(stream) != NULL && m->rdmemobj == m->wrmemobj &&
      __offset(stream) == __target(stream))
    {
      /* The right spot is already mapped.  */
      sio->accessed = 1;	/* Tell the FS we are reading it.  */
      __get_limit(stream) = __buffer(stream) + __bufsize(stream);
      if (!sio->written)
	/* Make the next writing operation call mapped_output,
	   so it can set the `written' bit at the right time.  */
	__put_limit(stream) = __buffer(stream);
      if (error = release_it(m))
	goto release_lost;
      __bufp(stream) = __buffer(stream);
      return *__bufp(stream)++;
    }

  if (__buffer(stream) != NULL)
    {
      /* Remove the old mapping.  */
      size_t mapping_size = ((__get_limit(stream) > __buffer(stream) ?
			      __get_limit(stream) : __put_limit(stream)) -
			     __buffer(stream));
      (void) __vm_deallocate(__mach_task_self(),
			     __buffer(stream), mapping_size);
      __buffer(stream) = NULL;
    }

  /* We're reading, so we're not at the end-of-file.  */
  __eof(stream) = 0;

  pos = __target(stream);
  while (sio->use_read_size && sio->read_size < pos)
    {
      if (sio->use_file_size && pos >= sio->file_size)
	{
	  /* Tried to read past the end of the file.  */
	  if (sio->eof_notify)
	    __io_eofnotify(m->file);
	  if (error = release_it(sio))
	    goto release_lost;
	  __eof(stream) = 1;
	  return EOF;
	}

      /* Block until there is more to read.  */
      error = __io_readsleep(m->file);
      if (error)
	{
	  (void) release_it(m);
	  errno = error;
	  return EOF;
	}
    }
  if (sio->use_read_size)
    size = sio->read_size;
  else if (sio->use_file_size)
    size = sio->file_size;
  else
    __libc_fatal("!use_read_size && !use_file_size\n");

  /* Round POS to a block boundary, leaving the excess in BUFP.  */
  bufp = pos % m->blksize;
  pos -= bufp;

  /* Decide how big a window on the file to use.  */
  size -= pos;
  if (size > m->blksize)
    size = m->blksize;

  /* Map the data.  */
  {
    vm_prot_t prot = VM_PROT_READ;
    if (stream->__mode.__write && m->rdmemobj == m->wrmemobj)
      prot |= VM_PROT_WRITE;
    error = __vm_map(__mach_task_self(),
		     &__buffer(stream), size, 0, 1,
		     m->rdmemobj, pos, 0, prot, prot, VM_INHERIT_NONE);
  }

  if (error == 0)
    /* Tell the FS that we have read some data.  */
    sio->accessed = 1;

  if (error == 0)
    (void) release_it(m);
  else
    error = release_it(m);

  if (error)
    {
    release_lost:
      __put_limit(stream) = __get_limit(stream) = __buffer(stream);
      errno = error;
      return EOF;
    }

  /* Set the offset to the position where our mapping begins.  */
  __offset(stream) = pos;
  /* Set the target position to just past the end of our mapping.  */
  __target(stream) = pos + size;

  /* Make the next output operation call __flshfp.  */
  __put_limit(stream) = __buffer(stream);

  __bufsize(stream) = size;
  __get_limit(stream) = __buffer(stream) + size;
  __bufp(stream) = __buffer(stream) + bufp;
  return (unsigned char) *__bufp(stream)++;
}

static void
DEFUN(mapped_output, (stream, c),
      FILE *stream AND int c)
{
  struct mapped *CONST m = (struct mapped *) cookie;
  struct shared_io *CONST sio = &m->shared;
  error_t error;
  off_t pos, bufp;
  size_t size;

  if (error = get_it(sio))
    {
      __error(stream) = 1;
      errno = error;
      return EOF;
    }

  if (__put_limit(stream) > __buffer(stream))
    {
      if (__bufp(stream) > __buffer(stream))
	/* Tell the FS that we have written some data.  */
	sio->written = 1;

      if (sio->use_postnotify_size &&
	  (__offset(stream) +
	   (__bufp(stream) - __buffer(stream))) > sio->postnotify_size)
	{
	  /* Notify the FS about what has been written.  */
	  if (error = __io_postnotify(m->file, m->cntl,
				      __offset(stream),
				      __bufp(stream) - __buffer(stream)))
	    goto end;
	}
    }

  if (__buffer(stream) != NULL)
    {
      /* Remove the old mapping.  */
      (void) __vm_deallocate(__mach_task_self(), __buffer(stream),
			     __get_limit(stream) - __buffer(stream));
      __buffer(stream) = NULL;
    }

  size = m->blksize;

  pos = __target(stream);
  /* Round POS to a block boundary, leaving the excess in BUFP.  */
  bufp = pos % size;
  pos -= bufp;

  if (sio->use_postnotify_size && size > sio->postnotify_size)
    /* %%% What if SIZE < BUFP after this?  */
    size = sio->postnotify_size;

  while (sio->use_prenotify_size && __target(stream) >= sio->prenotify_use)
    if (error = __io_prenotify(m->file, m->cntl, pos, size))
      goto end;

  /* Map the data.  */
  {
    vm_prot_t prot = VM_PROT_WRITE;
    if (stream->__mode.__read && m->wrmemobj == m->rdmemobj)
      prot |= VM_PROT_READ;
    error = __vm_map(__mach_task_self(),
		     &__buffer(stream), size, 0, 1,
		     m->memobj, pos, 0, prot, prot, VM_INHERIT_NONE);
  }

 end:
  release_it(m);

  if (error)
    {
      __put_limit(stream) = __get_limit(stream) = __buffer(stream);
      __error(stream) = 1;
      errno = error;
    }
  else
    {
      /* Set the offset to the position where our mapping begins.  */
      __target(stream) = __offset(stream) = pos;

      /* Make the next input operation call __fillbf.  */
      __get_limit(stream) = __buffer(stream);

      __bufsize(stream) = size;
      __put_limit(stream) = __buffer(stream) + size;
      __bufp(stream) = __buffer(stream) + bufp;
      *__bufp(stream)++ = (unsigned char) c;
    }
}

/* Initialize STREAM as necessary.
   This may change I/O functions, give a buffer, etc.
   If no buffer is allocated, but the bufsize is set,
   the bufsize will be used to allocate the buffer.  */
void
DEFUN(__stdio_init_stream, (stream), FILE *stream)
{
  CONST io_t file = (io_t) __cookie(stream);
  io_statbuf_t buf;
  memory_object_t cntl, xxmemobj, rdmemobj, wrmemobj;
  struct mapped *m;

  if (__io_stat(file, &buf))
    return;

  if (S_ISFIFO(buf.stb_mode))
    {
      /* It's named pipe (FIFO).  Make it unbuffered.  */
      __userbuf(stream) = 1;
      return;
    }

  if (sizeof(*m) > __vm_page_size)
    __libc_fatal("stdio: sizeof(struct mapped) > vm_page_size");

  /* Try to use mapped i/o.  */

  if (__io_map_cntl(m->file, &xxcntl, &rdcntl, &wtcntl))
    return;
  cntl = xxcntl; /* %%% ??? */

  if (__io_map(m->file, &xxmemobj, &rdmemobj, &wrmemobj))
    {
      (void) __port_deallocate(__mach_task_self(), cntl);
      return;
    }

  if (rdmemobj == MACH_PORT_NULL)
    rdmemobj = xxmemobj;
  if (wrmemobj == MACH_PORT_NULL)
    wrmemobj = xxmemobj;

  /* Lose if we can't do mapped i/o in the necessary direction(s).  */
  if ((stream->__mode.__read && rdmemobj == MACH_PORT_NULL) ||
      (stream->__mode.__write && wrmemobj == MACH_PORT_NULL) ||
      /* Map the shared page.  */
      __vm_map(__mach_task_self(), &m, sizeof(*m), NULL, 1, cntl, 0, 0,
	       VM_PROT_READ|VM_PROT_WRITE, VM_PROT_READ|VM_PROT_WRITE,
	       VM_INHERIT_NONE))
    {
      (void) __port_deallocate(__mach_task_self(), cntl);
      (void) __port_deallocate(__mach_task_self(), xxmemobj);
      (void) __port_deallocate(__mach_task_self(), rdmemobj);
      (void) __port_deallocate(__mach_task_self(), wrmemobj);
      return;
    }

  m->file = file;
  m->blksize = buf.stb_blksize;
  m->cntl = cntl;
  m->rdmemobj = rdmemobj;
  m->wrmemobj = wrmemobj;

  __io_funcs(stream).__close = mapped_close;
  __io_funcs(stream).__seek = mapped_seek;
  __room_funcs(stream).__input = mapped_input;
  __room_funcs(stream).__output = mapped_output;
  __cookie(stream) = (PTR) m;
  __userbuf(stream) = 1;	/* Tell stdio not to allocate a buffer.  */
}

#endif /* 0 */
