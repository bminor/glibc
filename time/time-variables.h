/* Global variables used by the time subsystem.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

/* This header should only be used within the time subsystem, but
   there are some historic external references.  The global variables
   declared here cannot be accessed in a thread-safe manner.  */

#ifndef TIME_VARIABLES_H
#define TIME_VARIABLES_H

#include <stdbool.h>
#include <time.h>

/* Global variables updated by tzset, localtime.  Their counterparts
   without the __ prefix are weak aliases of them.  The static linker
   will ensure that dynamically-linked programs which do not define
   their own variable (without the __ prefix) use the variable from
   glibc (which is updated by the glibc code through the __-prefixed
   alias).  If the application defines its own variable, it will be
   separate from the glibc variable.  */
extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;

/* Defined in localtime.c.  Used by localtime and gmtime.  */
extern struct tm _tmbuf attribute_hidden;

#endif /* TIME_VARIABLES_H */
