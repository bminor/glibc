/* Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

int
DEFUN(main, (argc, argv),
      int argc AND char **argv)
{
  if (argc > 1)
    {
      long int s[1025], d[1025];
      int i;
      int a, b = 0;

      a = atoi(argv[1]);
      if (argc > 2)
	b = atoi(argv[2]);

      for (i = 10000; --i; )
	memmove (((char *) d) + b, ((char *) s) + a, 1024 * 4);
    }
  else
    {
#define SIZE (3*0x200)

      unsigned char *p = valloc(SIZE + 40);
      int src_align, dst_align;
      int i;
      int len;

      for (len = 0; len < 256; len++)
	{
	  printf("phase %d\n", len);
	  for (src_align = 0; src_align < 32; src_align++)
	    {
	      unsigned char *src;

	      src = p + 40 + SIZE/3 + src_align;

	      for (dst_align = 0; dst_align < 32; dst_align++)
		{
		  unsigned char *dst;

		  memset (p, 0, SIZE + 40);
		  for (i = 0; i < len; i++)
		    src[i] = i;

		  dst = p + 40 + dst_align;
		  memmove (dst, src, len);

		  for (i = -32; i < 0; i++)
		    if (dst[i] != 0)
		      abort();
		  for (i = 0; i < len; i++)
		    if (dst[i] != i)
		      abort();
		  for (i = 32; i >= 0; i--)
		    if (dst[len + i] != 0)
		      abort();

		  memset (p, 0, SIZE + 40);
		  for (i = 0; i < len; i++)
		    src[i] = i;

		  dst = p + 40 + SIZE/3*2 + dst_align;
		  memmove (dst, src, len);

		  for (i = -32; i < 0; i++)
		    if (dst[i] != 0)
		      abort();
		  for (i = 0; i < len; i++)
		    if (dst[i] != i)
		      abort();
		  for (i = 32; i >= 0; i--)
		    if (dst[len + i] != 0)
		      abort();
		}
	    }
	}
    }

  exit (0);
}
