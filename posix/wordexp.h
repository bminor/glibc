/* Copyright (C) 1990 Free Software Foundation, Inc.
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

#ifndef	_WORDEXP_H

#define	_WORDEXP_H	1
#include <features.h>

/* Bits set in the FLAGS argument to `wordexp'.  */
enum
  {
    WRDE_DOOFFS	= (1 << 0),	/* Insert PWORDEXP->we_offs NULLs.  */
    WRDE_APPEND	= (1 << 1),	/* Append to results of a previous call.  */
    WRDE_NOCMD	= (1 << 2),	/* Don't do command substitution.  */
    __WRDE_FLAGS = (WRDE_DOOFFS|WRDE_APPEND|WRDE_NOCMD),
  };

/* Structure describing a word-expansion run.  */
typedef struct
  {
    int we_wordc;	/* Count of words matched.  */
    char **we_wordv;	/* List of expanded words.  */
    int we_offs;	/* Slots to reserve in `we_wordv'.  */
  } wordexp_t;

/* Do word expansion of WORDS into PWORDEXP.  */
extern int EXFUN(wordexp, (CONST char *__words, wordexp_t *__pwordexp,
			   int __flags));

/* Free the storage allocated by a `wordexp' call.  */
extern void EXFUN(wordfree, (wordexp_t *));

#endif	/* wordexp.h  */
