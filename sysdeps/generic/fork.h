/* Stub version of header for fork handling.  Mainly to handle pthread_atfork
   and friends.  Outside dependencies:

   UNREGISTER_ATFORK
     If defined it must expand to a function call which takes one void*
     parameter which is the DSO handle for the DSO which gets unloaded.
     The function so called has to remove the atfork handlers registered
     by this module.  */


/* System specific fork definition.  Generic version.
   Copyright (C) 2002-2021 Free Software Foundation, Inc.
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
