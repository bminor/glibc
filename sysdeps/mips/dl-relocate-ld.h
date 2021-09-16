/* Check if dynamic section should be relocated.  MIPS version.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#ifndef _DL_RELOCATE_LD_H
#define _DL_RELOCATE_LD_H

/* Return true if dynamic section in the shared library L should be
   relocated.  */

static inline bool
dl_relocate_ld (const struct link_map *l)
{
  /* Never relocate dynamic section.  */
  return false;
}

#endif /* _DL_RELOCATE_LD_H */
