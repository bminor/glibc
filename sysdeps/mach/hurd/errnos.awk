# Copyright (C) 1991 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# errno.texinfo contains lines like:
# @deftypevr Macro int ENOSYS     @c 78 Function not implemented

BEGIN
  {
    print "/* This file is generated from errno.texinfo by errnos.awk. */";
    print "";
    print "#ifdef _ERRNO_H";
  }
$1 == "@deftypevr" && $5 == "@c"
  {
    if ($4 == "EDOM" || $4 == "ERANGE")
      {
        print "#endif /* <errno.h> included.  */";
	print "#if (!defined (__Emath_defined) && \\\n\
     (defined (_ERRNO_H) || defined (__need_Emath)))";
      }
     printf "#define\t%s\t%d\t/* ", $4, $6;
     for (i = 7; i < NF; ++i)
       printf "%s ", $i
     print " */"
    if ($4 == "EDOM" || $4 == "ERANGE")
      {
        print "#endif /* Emath not defined and <errno.h> \
included or need Emath.  */";
	print "#ifdef _ERRNO_H"
      }
  }
END
  {
    print "#endif /* <errno.h> included.  */"
  }
