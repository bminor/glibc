/* Copyright (C) 2019 Free Software Foundation, Inc.
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

/* This header is internal to glibc and should not be included outside
   of glibc headers.  It is included by each header that needs to make
   global declarations that may or may not already have been made by a
   header provided by the underlying operating system kernel.  All such
   declarations are associated with macros named __UAPI_DEF_something,
   which have three possible values:

   If __UAPI_DEF_FOO is not defined, nobody has yet declared `foo'.
   If __UAPI_DEF_FOO is defined to 0, glibc's headers have declared `foo'.
   If __UAPI_DEF_FOO is defined to 1, the kernel's headers have declared `foo`.

   This header cannot have a multiple-inclusion guard, because it needs
   to recheck for additional declarations by kernel headers each time
   a glibc header that uses it is included.

   This generic version of uapi-compat.h is used on operating systems
   where none of the above coordination is necessary.  */
