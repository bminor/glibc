/* Optional code to distinguish library flavours.
   Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Peter Bergner <bergner@linux.vnet.ibm.com>

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

#ifndef _DL_LIBRECON_H

#include <sysdeps/unix/sysv/linux/dl-librecon.h>

/* Recognizing extra environment variables.  */
#define EXTRA_LD_ENVVARS_11 \
  if (memcmp (envline, "AT_PLATFORM", 11) == 0)				      \
    {									      \
      GLRO(dl_platform) = &envline[12];					      \
      /* Determine the length of the platform name.  */			      \
      if (GLRO(dl_platform) != NULL)					      \
        GLRO(dl_platformlen) = strlen (GLRO(dl_platform));		      \
      break;		 		 		 		      \
    }

#endif /* dl-librecon.h */
