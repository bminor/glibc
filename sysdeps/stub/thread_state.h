/* Mach thread state definitions for Hurd code.  Stub version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

/* Everything else is called `thread_state', but CMU's header file is
   called `thread_status'.  Oh boy.  */
#include <mach/thread_state.h>

/* Replace <machine> with "i386" or "mips" or whatever.  */

#define HURD_THREAD_STATE_FLAVOR	<machine>_THREAD_STATE
#define HURD_THREAD_STATE_COUNT		<machine>_THREAD_STATE_COUNT

#define hurd_thread_state <machine>_thread_state

/* Define these to the member names in `struct <machine>_thread_state'
   for the PC and stack pointer.  */
#define PC ?
#define SP ?
