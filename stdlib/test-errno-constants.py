# Test that errno_constants.py includes every error number defined by errno.h.
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

# Usage: test-errno-constants.py $(common-objpfx)stdlib $(CC) $(CFLAGS) $(CPPFLAGS)

import os
import sys
import subprocess

sys.path.append(sys.argv[1])
import errno_constants

def main():
    cc_cmd = sys.argv[2:]
    cc_cmd.extend(["-E", "-dM", "-xc", "-"])
    cc_proc = subprocess.Popen(cc_cmd,
                               stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE)
    cc_proc.stdin.write(b"#define _GNU_SOURCE\n"
                       b"#include <errno.h>\n")
    cc_proc.stdin.close()

    cc_output = cc_proc.stdout.read()
    status = cc_proc.wait()
    if status:
        sys.stderr.write("{}\nunsuccessful exit, status {:04x}"
                         .format(" ".join(cc_cmd), status))
        sys.exit(1)

    ok = True
    for line in cc_output.decode("utf-8").splitlines():
        if not line.startswith("#define E"):
            continue
        emacro = line.split()[1]
        if not hasattr(errno_constants, emacro):
            if ok:
                sys.stderr.write("*** Missing constants:\n")
                ok = False
            sys.stderr.write(emacro + "\n")

    sys.exit(0 if ok else 1)

main()
