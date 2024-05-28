# CPU diagnostics probing.  Generating documentatable for t he manual
# Copyright (C) 2024 Free Software Foundation, Inc.
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
# <https://www.gnu.org/licenses/>.

import re
import sys

path, = sys.argv[1:]

RE_PROBE = re.compile('^\s+_dl_x86_probe_([a-z0-9_]+)\);$')

bit = 1
with open(path) as inp:
    for line in inp:
        m = RE_PROBE.match(line)
        if m:
            name = m.group(1)
            print('@item 0x{:08x}:{:08x}'.format(bit >> 32, bit & 0xffffffff))
            print('@code{' + name + '}')
            bit *= 2
