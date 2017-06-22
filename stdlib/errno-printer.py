# Pretty printer for errno.
# Copyright (C) 2016-2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

"""This file contains the gdb pretty printers for the following types:

    * __error_t (the type of 'errno')
    * error_t   (cast any 'int' to 'error_t' to print it like an errno value)

You can check which printers are registered and enabled by issuing the
'info pretty-printer' gdb command.  Printers should trigger automatically when
trying to print a variable of one of the types mentioned above.
"""


import gdb
import gdb.printing
import errno_constants


def make_errno_reverse_mapping():
    """Construct a reverse mapping from errno values to symbolic names.
       The result is a dictionary indexed by integers, not a list,
       because errno values are not necessarily contiguous.
    """

    # Certain errno symbols are allowed to have the same numeric value.
    # If they do, one of them (whichever one is in POSIX, or if both or
    # neither are, the shortest) is selected as the preferred name.
    # This map goes from non-preferred name(s) to preferred name.
    permitted_collisions = {
        "EDEADLOCK":   "EDEADLK",
        "EOPNOTSUPP":  "ENOTSUP",
        "EWOULDBLOCK": "EAGAIN",
    }

    errno_names = { 0: "Success" }
    for name in dir(errno_constants):
        if name[0] == 'E':
            number = getattr(errno_constants, name)
            other = errno_names.get(number)
            if other is None:
                errno_names[number] = name
            else:
                p1 = permitted_collisions.get(name)
                p2 = permitted_collisions.get(other)
                if p1 is not None and p1 == other:
                    pass # the value in errno_names is already what we want
                elif p2 is not None and p2 == name:
                    errno_names[number] = name
                else:
                    raise RuntimeError(
                        "errno value collision: {} = {}, {}"
                        .format(number, name, errno_names[number]))

    return errno_names


errno_names = make_errno_reverse_mapping()


class ErrnoPrinter(object):
    """Pretty printer for errno values."""

    def __init__(self, val):
        self._val = int(val)

    def to_string(self):
        """gdb API function.

        This is called from gdb when we try to print an error_t.
        """
        if self._val in errno_names:
            return "{:d} ({})".format(self._val, errno_names[self._val])
        else:
            return "{:d}".format(self._val)


def register(objfile):
    """Register pretty printers for the current objfile."""

    printer = gdb.printing.RegexpCollectionPrettyPrinter("glibc-errno")
    printer.add_printer('error_t', r'^(?:__)?error_t', ErrnoPrinter)

    if objfile == None:
        objfile = gdb

    gdb.printing.register_pretty_printer(objfile, printer)


register(gdb.current_objfile())
