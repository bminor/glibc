# Copyright (C) 2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# Generate errno_constants.pysym from errno.texi.
# errno.texi contains lines like:
# @errno{ENOSYS, 123, Function not implemented}
# The number is only relevant for the Hurd.

BEGIN {
    print "#include <errno.h>"
    print ""
    print "-- Errno constants"

    # Some error constants do not exist on all supported operating systems.
    # FIXME: Encode this information in errno.texi.
    ## (Sometimes) two names for the same number
    optional["EDEADLOCK"]	= 1
    optional["EDEADLK"]		= 1
    optional["EOPNOTSUPP"]	= 1
    optional["ENOTSUP"]		= 1
    optional["EWOULDBLOCK"]	= 1
    optional["EAGAIN"]		= 1
    ## BSD-specific
    optional["EAUTH"]		= 1
    optional["EBADRPC"]		= 1
    optional["EFTYPE"]		= 1
    optional["ENEEDAUTH"]	= 1
    optional["EPROCLIM"]	= 1
    optional["EPROCUNAVAIL"]	= 1
    optional["EPROGMISMATCH"]	= 1
    optional["EPROGUNAVAIL"]	= 1
    optional["ERPCMISMATCH"]	= 1
    ## GNU-specific
    optional["EBACKGROUND"]	= 1
    optional["ED"]		= 1
    optional["EDIED"]		= 1
    optional["EGRATUITOUS"]	= 1
    optional["EGREGIOUS"]	= 1
    optional["EIEIO"]		= 1
}

/^@errno\{/ {
    e = substr($1, 8, length($1)-8)
    if (e in optional)
      {
	print "#ifdef", e
	print e
	print "#endif"
      }
    else
	print e
}
