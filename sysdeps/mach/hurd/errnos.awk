# Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
# @comment errno.h
# @comment POSIX.1: Function not implemented
# @deftypevr Macro int ENOSYS

BEGIN {
    print "/* This file is generated from errno.texinfo by errnos.awk. */";
    print "";
    print "#ifdef _ERRNO_H\n";
    print "/* The Hurd uses Mach error system 0x10, currently only subsystem 0. */"
    print "#define _HURD_ERRNO(n)\t((0x10 << 26) | ((n) & 0x3fff))";
    errno = 0;
    errnoh = 0;
  }
$1 == "@comment" && $2 == "errno.h" { errnoh=1; next }
$1 == "@comment" && errnoh == 1 \
  {
    ++errnoh;
    etext = "";
    for (i = 3; i <= NF; ++i)
      etext = etext " " $i;
    next;
  }
errnoh == 2 && $1 == "@deftypevr"  && $2 == "Macro" && $3 == "int" \
  {
    e = $4;
    if (e == "EDOM" || e == "ERANGE")
      {
	print "#endif /* <errno.h> included.  */";
	print "#if (!defined (__Emath_defined) && (defined (_ERRNO_H) || defined (__need_Emath)))";\
      }
    s = "#define\t" e;
    l = 24 - length (s);
    while (l-- > 0)
      s = s " ";
    printf "%s_HURD_ERRNO (%d)\t/*%s */\n", s, ++errno, etext;
    if (e == "EDOM" || e == "ERANGE")
      {
	print "#endif /* Emath not defined and <errno.h> included or need Emath.  */";
	print "#ifdef _ERRNO_H";
      }
    next;
  }
{ errnoh=0 }
END {
  printf "#define _HURD_ERRNOS %d\n", ++errno;
  print "#endif /* <errno.h> included.  */"
}
