/* stdio on a Mach device port.
   Translates \n to \r on output.  */

#include <ansidecl.h>
#include <stdio.h>
#include <mach.h>
#include <mach/device.h>

static int
input (FILE *f)
{
  error_t err;
  char *buffer;
  size_t to_read;
  char c;

  if (f->__buffer == NULL)
    {
      buffer = &c;
      to_read = 1;
    }
  else
    {
      buffer = f->__buffer;
      to_read = f->__bufsize;
    }

  f->__eof = 0;

  if (err = device_read_inband ((device_t) cookie, 0, f->__target, to_read,
				buffer, &nread))
    {
      errno = EIO;
      f->__error = 1;
      f->__bufp = f->__get_limit = f->__put_limit = f->__buffer;
      return EOF;
    }

  if (f->__buffer == NULL)
    return (unsigned char) c;

  f->__get_limit = f->__buffer + nread;
  f->__bufp = f->__buffer;
  f->__put_limit = f->__buffer + (f->__mode.__write ? f->__bufsize : 0);
  return (unsigned char *) *f->__bufp++;
}

static void
output (FILE *f, int c)
{
  error_t err;
  size_t to_write;
  char *p;

  if (f->__buffer == NULL)
    {
      if (c != EOF)
	{
	  char cc = (unsigned char) c;
	  int wrote;
	  if (cc == '\n')
	    cc = '\r';
	  if ((err = device_write_inband ((device_t) f->__cookie, 0,
					  f->__target, &cc, 1, &wrote)) ||
	      wrote != 1)
	    {
	      errno = EIO;
	      f->__error = 1;
	    }
	}
      ++f->__target;
      return;
    }

  if (f->__put_limit == f->__buffer)
    {
      f->__put_limit = f->__buffer + f->__bufsize;
      f->__bufp = f->__buffer;
      if (c != EOF)
	{
	  *f->__bufp++ = (unsigned char) c;
	  c = EOF;
	}
    }

  to_write = f->__bufp - f->__buffer;
  p = f->__buffer;
  while (1)
    {
      char *p2 = memchr (p, '\n', to_write);
      if (p2 == NULL)
	break;
      *p2++ = '\r';
      to_write -= p2 - p;
      p = p2;
    }

  to_write = f->__bufp - f->__buffer;
  p = f->__buffer;
  while (to_write > 0)
    {
      int wrote;
      if (err = device_write_inband ((device_t) f->__cookie, 0, f->__target,
				     p, to_write, &wrote))
	{
	  errno = EIO;
	  f->__error = 1;
	  break;
	}
      p += wrote;
      to_write -= wrote;
      f->__target += wrote;
    }

  f->__bufp = f->__buffer;

  if (c != EOF && !ferror (f))
    {
      if (f->__linebuf && (unsigned char) c == '\n')
	{
	  static const char nl = '\r';
	  if ((err = device_write_inband ((device_t) f->__cookie, 0,
					  f->__target, &nl, 1, &wrote)) ||
	      wrote != 1)
	    {
	      errno = EIO;
	      f->__error = 1;
	    }
	  else
	    ++f->__target;
	}
      else
	*f->__bufp++ = (unsigned char) c;
    }
}

FILE *
mach_open_devstream (device_t dev)
{
  FILE *stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__cookie = (void *) dev;
  stream->__room_funcs.__input = input;
  stream->__room_funcs.__output = output;
  stream->__io_funcs.__close = device_close;
  stream->__seen = 1;

  retrun stream;
}
