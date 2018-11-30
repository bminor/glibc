#!/usr/bin/python3
# Produce headers of assembly constants from C expressions.
# Copyright (C) 2018 Free Software Foundation, Inc.
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

# The input to this script looks like:
#       #cpp-directive ...
#       NAME1
#       NAME2 expression ...
# A line giving just a name implies an expression consisting of just that name.

import argparse
import os.path
import re
import subprocess
import tempfile


def compute_c_consts(sym_data, cc):
    """Compute the values of some C constants.

    The first argument is a list whose elements are either strings
    (preprocessor directives) or pairs of strings (a name and a C
    expression for the corresponding value).  Preprocessor directives
    in the middle of the list may be used to select which constants
    end up being evaluated using which expressions.

    """
    out_lines = []
    started = False
    for arg in sym_data:
        if isinstance(arg, str):
            out_lines.append(arg)
            continue
        name = arg[0]
        value = arg[1]
        if not started:
            out_lines.append('void\ndummy (void)\n{')
            started = True
        out_lines.append('asm ("@@@name@@@%s@@@value@@@%%0@@@end@@@" '
                         ': : \"i\" ((long int) (%s)));'
                         % (name, value))
    if started:
        out_lines.append('}')
    out_lines.append('')
    out_text = '\n'.join(out_lines)
    with tempfile.TemporaryDirectory() as temp_dir:
        c_file_name = os.path.join(temp_dir, 'test.c')
        s_file_name = os.path.join(temp_dir, 'test.s')
        with open(c_file_name, 'w') as c_file:
            c_file.write(out_text)
        # Compilation has to be from stdin to avoid the temporary file
        # name being written into the generated dependencies.
        cmd = ('%s -S -o %s -x c - < %s' % (cc, s_file_name, c_file_name))
        subprocess.check_call(cmd, shell=True)
        consts = {}
        with open(s_file_name, 'r') as s_file:
            for line in s_file:
                match = re.search('@@@name@@@([^@]*)'
                                  '@@@value@@@[^0-9Xxa-fA-F-]*'
                                  '([0-9Xxa-fA-F-]+).*@@@end@@@', line)
                if match:
                    if (match.group(1) in consts
                        and match.group(2) != consts[match.group(1)]):
                        raise ValueError('duplicate constant %s'
                                         % match.group(1))
                    consts[match.group(1)] = match.group(2)
        return consts


def gen_test(sym_data):
    """Generate a test for the values of some C constants.

    The first argument is as for compute_c_consts.

    """
    out_lines = []
    started = False
    for arg in sym_data:
        if isinstance(arg, str):
            out_lines.append(arg)
            continue
        name = arg[0]
        value = arg[1]
        if not started:
            out_lines.append('#include <stdint.h>\n'
                             '#include <stdio.h>\n'
                             '#include <bits/wordsize.h>\n'
                             '#if __WORDSIZE == 64\n'
                             'typedef uint64_t c_t;\n'
                             '# define U(n) UINT64_C (n)\n'
                             '#else\n'
                             'typedef uint32_t c_t;\n'
                             '# define U(n) UINT32_C (n)\n'
                             '#endif\n'
                             'static int\n'
                             'do_test (void)\n'
                             '{\n'
                             # Compilation test only, using static assertions.
                             '  return 0;\n'
                             '}\n'
                             '#include <support/test-driver.c>')
            started = True
        out_lines.append('_Static_assert (U (asconst_%s) == (c_t) (%s), '
                         '"value of %s");'
                         % (name, value, name))
    return '\n'.join(out_lines)


def main():
    """The main entry point."""
    parser = argparse.ArgumentParser(
        description='Produce headers of assembly constants.')
    parser.add_argument('--cc', metavar='CC',
                        help='C compiler (including options) to use')
    parser.add_argument('--test', action='store_true',
                        help='Generate test case instead of header')
    parser.add_argument('sym_file',
                        help='.sym file to process')
    args = parser.parse_args()
    sym_data = []
    with open(args.sym_file, 'r') as sym_file:
        for line in sym_file:
            line = line.strip()
            if line == '':
                continue
            # Pass preprocessor directives through.
            if line.startswith('#'):
                sym_data.append(line)
                continue
            words = line.split(maxsplit=1)
            # Separator.
            if words[0] == '--':
                continue
            name = words[0]
            value = words[1] if len(words) > 1 else words[0]
            sym_data.append((name, value))
    if args.test:
        print(gen_test(sym_data))
    else:
        consts = compute_c_consts(sym_data, args.cc)
        print('\n'.join('#define %s %s' % c for c in sorted(consts.items())))

if __name__ == '__main__':
    main()
