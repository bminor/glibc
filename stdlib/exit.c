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
#include <stdlib.h>
#include <unistd.h>
#include "exit.h"

#ifdef	__GNU_STAB__
CONST unsigned long int __libc_atexit[1];
#endif

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered
   perform stdio cleanup, and terminate program execution with STATUS.  */
__NORETURN
void
DEFUN(exit, (status), int status)
{
  register CONST struct exit_function_list *l;

  for (l = __exit_funcs; l != NULL; l = l->next)
    {
      register size_t i = l->idx;
      while (i-- > 0)
	{
	  CONST struct exit_function *CONST f = &l->fns[i];
	  switch (f->flavor)
	    {
	    case ef_free:
	      break;
	    case ef_on:
	      (*f->func.on.fn)(status, f->func.on.arg);
	      break;
	    case ef_at:
	      (*f->func.at)();
	      break;
	    }
	}
    }

#ifdef	__GNU_STAB__
  {
    register unsigned long int i;
    for (i = 1; i <= __libc_atexit[0]; ++i)
      (*(void EXFUN((*), (NOARGS))) __libc_atexit[i])();
  }
#else
  {
    extern void EXFUN(_cleanup, (NOARGS));
    _cleanup();
  }
#endif

  _exit(status);
}

