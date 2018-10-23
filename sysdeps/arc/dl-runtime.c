/* dl-runtime helpers for ARC.
   Copyright (C) 2017-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* PLT jump into resolver passes PC of PLTn, while _dl_fixup expects the
   address of corresponding .rela.plt entry.  */

#ifdef __A7__
# define ARC_PLT_SIZE	12
#else
# define ARC_PLT_SIZE	16
#endif

#define reloc_index						\
({								\
  unsigned long int plt0 = D_PTR (l, l_info[DT_PLTGOT]);	\
  unsigned long int pltn = reloc_arg;				\
  /* Exclude PL0 and PLT1.  */					\
  unsigned long int idx = (pltn - plt0)/ARC_PLT_SIZE - 2;	\
  idx;								\
})

#define reloc_offset reloc_index * sizeof (PLTREL)

#include <elf/dl-runtime.c>
