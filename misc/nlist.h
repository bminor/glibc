/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef	_NLIST_H

#define	_NLIST_H	1
#include <features.h>


/* Structure describing a symbol-table entry.  */
struct nlist
  {
    char *n_name;
    unsigned char n_type;
    char n_other;
    short int n_desc;
    unsigned long int n_value;
  };
#define	N_NLIST_DECLARED
#include <a.out.h>


/* Search the executable FILE for symbols matching those in NL,
   which is terminated by an element with a NULL `n_un.n_name' member,
   and fill in the elements of NL.  */
extern int EXFUN(nlist, (CONST char *__file, struct nlist *__nl));


#endif	/* nlist.h  */
