/* Using NaCl interface tables.
   Copyright (C) 2013 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _NACL_INTERFACES_H
#define _NACL_INTERFACES_H	1

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <ldsodefs.h>

#include <native_client/src/untrusted/irt/irt.h>

/* This is how we access the IRT interface-query function.
   This formulation makes it usable as if it were a function name.  */
#define __nacl_irt_query	(*(TYPE_nacl_irt_query) GLRO(dl_sysinfo))


/* This describes one IRT (or IRT-like) interface that libc uses.
   This structure contains no pointers, so it can go into rodata
   without relocs.  Instead, the actual tables we use for these
   interfaces live in a parallel section in writable data.  */
struct nacl_interface {
  size_t table_size;
  size_t namelen;
  char name[];
};

/* Increment for 'const struct nacl_interface *' pointers.  */
static inline const struct nacl_interface *
next_nacl_interface (const struct nacl_interface *i)
{
  uintptr_t align = __alignof (*i);
  return (const void *) (((uintptr_t) &i->name[i->namelen] + align - 1)
                         & -align);
}

#define NACL_MANDATORY_INTERFACE(id, type)	extern struct type __##type;
#define NACL_OPTIONAL_INTERFACE(id, type)	extern struct type __##type;
#include "nacl-interface-list.h"
#undef	NACL_MANDATORY_INTERFACE
#undef	NACL_OPTIONAL_INTERFACE

extern void __nacl_initialize_interfaces (void) attribute_hidden;

/* Convenience function for handling IRT call return values.  */
static inline int
__nacl_fail (int err)
{
  errno = err;
  return -1;
}

#define NACL_CALL(err, val) \
  ({ int _err = (err); _err ? __nacl_fail (_err) : (val); })

#endif  /* nacl-interfaces.h */
