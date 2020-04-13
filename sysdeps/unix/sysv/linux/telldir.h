/* Linux internal telldir definitions.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#ifndef _TELLDIR_H
#define _TELLDIR_H 1

#include <dirent.h>

#if _DIRENT_OFFSET_TRANSLATION

_Static_assert (sizeof (long int) < sizeof (__off64_t),
		"sizeof (long int) >= sizeof (__off64_t)");

# include <intprops.h>

/* On platforms where 'long int' is smaller than 'off64_t' this is how the
   returned value is encoded and returned by 'telldir'.  If the directory
   offset can be enconded in 31 bits it is returned in the 'info' member
   with 'is_packed' set to 1.

   Otherwise, the 'info' member describes an index in a dynamic array at
   'DIR' structure.  */

union dirstream_packed
{
  long int l;
  struct
  {
    unsigned long int is_packed:1;
    unsigned long int info:31;
  } p;
};

/* telldir maintains a list of offsets that describe the obtained diretory
   position if it can fit this information in the returned 'dirstream_packed'
   struct.  */

# define DYNARRAY_STRUCT  dirstream_loc_t
# define DYNARRAY_ELEMENT off64_t
# define DYNARRAY_PREFIX  dirstream_loc_
# include <malloc/dynarray-skeleton.c>

static __always_inline bool
telldir_need_dirstream (__off64_t d_off)
{
  return ! (TYPE_MINIMUM (off_t) <= d_off && d_off <= TYPE_MAXIMUM (off_t));
}
#else

_Static_assert (sizeof (long int) == sizeof (off64_t),
		"sizeof (long int) != sizeof (off64_t)");

#endif /* __LP64__  */

#endif /* _TELLDIR_H  */
