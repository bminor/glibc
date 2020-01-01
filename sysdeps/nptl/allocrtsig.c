/* Handle real-time signal allocation.  NPTL version.
   Copyright (C) 2015-2020 Free Software Foundation, Inc.
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

#include <signal.h>
#include <nptl/pthreadP.h>

#if SIGTIMER != SIGCANCEL
# error "SIGTIMER and SIGCANCEL must be the same"
#endif

/* This tells the generic code (included below) how many signal
   numbers need to be reserved for libpthread's private uses
   (SIGCANCEL and SIGSETXID).  */
#define RESERVED_SIGRT 2

#include <signal/allocrtsig.c>
