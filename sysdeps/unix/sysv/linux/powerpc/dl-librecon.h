/* Optional code to distinguish library flavors.
   Copyright (C) 2010-2012 Free Software Foundation, Inc.
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

#ifndef _DL_LIBRECON_H

#include <string.h>
#include <sysdeps/unix/sysv/linux/dl-librecon.h>

/* Recognizing extra environment variables.  */
#define EXTRA_LD_ENVVARS_11 \
  if (memcmp (envline, "AT_PLATFORM", 11) == 0)				      \
    {									      \
      int platformlen = strlen (&envline[12]);				      \
      GLRO(dl_platformlen) = platformlen;				      \
      if (platformlen > 0)						      \
	{								      \
	  GLRO(dl_platform) = &envline[12];				      \
	  break;							      \
	}								      \
      GLRO(dl_platform) = NULL;						      \
      break;		 		 		 		      \
    }

/* Extra unsecure variables.  The names are all stuffed in a single
   string which means they have to be terminated with a '\0' explicitly.  */
#define EXTRA_UNSECURE_ENVVARS \
  "LD_AT_PLATFORM\0"

#endif /* dl-librecon.h */
