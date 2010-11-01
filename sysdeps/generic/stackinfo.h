/* Copyright (C) 1999, 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This file contains generic information about the stack allocation.  */

#ifndef _STACKINFO_H
#define _STACKINFO_H	1
#include <elf.h>

/* Default to an executable stack.  PF_X can be overridden if PT_GNU_STACK is
 * present, but it is presumed absent.  */
#define _STACK_FLAGS	(PF_R|PF_W|PF_X)

#endif /* stackinfo.h  */
