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
    print "/* This file is generated from errno.texi by errlist.awk.  */"
    print "";
    print "#ifndef HAVE_GNU_LD"
    print "#define _sys_nerr sys_nerr"
    print "#define _sys_errlist sys_errlist"
    print "#endif"
    print ""
    print "const char *_sys_errlist[] =";
    print "  {";
    errno = 0;
    print "    \"Success\","
  }
$1 == "@comment" && $2 == "errno.h" { errnoh=1; next }
errnoh == 1 && $1 == "@comment" \
  {
    ++errnoh;
    etext = $3;
    for (i = 4; i <= NF; ++i)
      etext = etext " " $i;
    next;
  }
errnoh == 2 && $1 == "@deftypevr"  && $2 == "Macro" && $3 == "int" \
  {
    e = $4;
    s = "    \"" etext "\",";
    l = 40 - length (s);
    while (l-- > 0)
      s = s " ";
    printf "%s/* %d = %s */\n", s, ++errno, e;
    next;
  }
{ errnoh=0 }
END {
  print "  };";
  print "";
  printf "const int _sys_nerr = %d;\n", errno;
  }
