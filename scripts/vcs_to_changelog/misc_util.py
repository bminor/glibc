# General Utility functions.
# Copyright (C) 2019-2020 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys

class DebugUtil:
    debug = False
    def __init__(self, debug):
        self.debug = debug

    def eprint(self, *args, **kwargs):
        ''' Print to stderr.
        '''
        print(*args, file=sys.stderr, **kwargs)


    def print(self, *args, **kwargs):
        ''' Convenience function to print diagnostic information in the program.
        '''
        if self.debug:
            self.eprint(*args, **kwargs)


def decode(string):
    ''' Attempt to decode a string.

    Decode a string read from the source file.  The multiple attempts are needed
    due to the presence of the page break characters and some tests in locales.
    '''
    codecs = ['utf8', 'cp1252']

    for i in codecs:
        try:
            return string.decode(i)
        except UnicodeDecodeError:
            pass

    DebugUtil.eprint('Failed to decode: %s' % string)
