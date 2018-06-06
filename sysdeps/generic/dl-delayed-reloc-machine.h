/* Delayed relocation processing.  Generic version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#ifndef _DL_DELAYED_RELOC_MACHINE_H
#define _DL_DELAYED_RELOC_MACHINE_H

/* Process a delayed relocation.  In the default implementation, there
   are no delayed relocations, so this implementation does
   nothing.  */
static inline void
_dl_delayed_reloc_machine (const struct dl_delayed_reloc *reloc)
{
}

#endif  /* _DL_DELAYED_RELOC_MACHINE_H */
