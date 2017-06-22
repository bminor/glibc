# Test for the errno pretty-printer.
# Copyright (C) 2017 Free Software Foundation, Inc.
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

import sys

from test_printers_common import *

test_source = sys.argv[1]
test_bin = sys.argv[2]
printer_files = sys.argv[3:]
printer_names = ['global glibc-errno']

try:
    init_test(test_bin, printer_files, printer_names)
    go_to_main()

    # All supported versions of gdb do run the pretty-printer on an
    # _array_ of error_t.
    test_printer('array_of_error_t',
                 r'= \{0 \(Success\), \d+ \(ERANGE\), -2\}', is_ptr=False)

    # Some versions of gdb don't run the pretty-printer on a _scalar_
    # whose type is error_t.  If we have such a gdb, the test is
    # unsupported.
    test('print (error_t) 0',
         pattern             = r'= 0 \(Success\)$',
         unsupported_pattern = r'= 0$')

    # Some versions of gdb don't support reading thread-specific variables;
    # these versions may also have trouble reading errno.
    test('print ensure_gdb_can_read_thread_variables',
         pattern             = r'= 0$',
         unsupported_pattern = r'Cannot find thread-local')

    next_cmd()
    next_cmd()
    test_printer('errno', r'0 (Success)', is_ptr=False)
    next_cmd()
    test_printer('errno', r'\d+ (ERANGE)', is_ptr=False)

    break_at(test_source, 'test errno 2', is_ptr=False)
    continue_cmd()
    next_cmd()
    test_printer('errno', r'-2', is_ptr=False)

    continue_cmd() # Exit

except (NoLineError, pexpect.TIMEOUT) as exception:
    print('Error: {0}'.format(exception))
    result = FAIL

else:
    print('Test succeeded.')
    result = PASS

exit(result)
