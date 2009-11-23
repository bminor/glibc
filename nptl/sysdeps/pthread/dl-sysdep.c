/* libthread_db hooks in the dynamic linker.
   Copyright (C) 2009 Free Software Foundation, Inc.
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

/* This cpp magic lets us use #include_next in the #else branch.  */
#ifndef IS_IN_dl_sysdep
# define IS_IN_dl_sysdep 1
# include <dl-sysdep.c>
#else

# include_next <dl-sysdep.c>

/* A dynamic linker with TLS support needs to make some information
   available to libthread_db so debuggers can figure out TLS lookups
   even when libpthread is not loaded.  */

# include <version.h>

const char _thread_db_dl_nptl_version[] __attribute_used__ = VERSION;

# include <../nptl_db/db_info.c>

#endif
