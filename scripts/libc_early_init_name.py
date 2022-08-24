#!/usr/bin/python3
# Compute the hash-based name of the __libc_early_init function.
# Copyright (C) 2022 Free Software Foundation, Inc.
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

"""Compute the name of the __libc_early_init function, which is used
as a protocol version marker between ld.so and libc.so.

The name contains a hash suffix, and the hash changes if certain key
files in the source tree change.  Distributions can also configure
with --with-extra-version-id, to make the computed hash dependent on
the package version.

"""

import argparse
import hashlib
import os
import string
import sys

def gnu_hash(s):
    """Computes the GNU hash of the string."""
    h = 5381
    for ch in s:
        if type(ch) is not int:
            ch = ord(ch)
        h = (h * 33 + ch) & 0xffffffff
    return h

# Parse the command line.
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', metavar='PATH',
                    help='path to header file this tool generates')
parser.add_argument('--extra-version-id', metavar='ID',
                    help='extra string to influence hash computation')
parser.add_argument('inputs', metavar='PATH', nargs='*',
                    help='files whose contents influences the generated hash')
opts = parser.parse_args()

# Obtain the blobs that affect the generated hash.
blobs = [(opts.extra_version_id or '').encode('UTF-8')]
for path in opts.inputs:
    with open(path, 'rb') as inp:
        blobs.append(inp.read())

# Hash the file boundaries.
md = hashlib.sha256()
md.update(repr([len(blob) for blob in blobs]).encode('UTF-8'))

# And then hash the file contents.  Do not hash the paths, to avoid
# impacting reproducibility.
for blob in blobs:
    md.update(blob)

# These are the bits used to compute the suffix.
derived_bits = int.from_bytes(md.digest(), byteorder='big', signed=False)

# These digits are used in the suffix (should result in base-62 encoding).
# They must be valid in C identifiers.
digits = string.digits + string.ascii_letters

# Generate eight digits as a suffix.  They should provide enough
# uniqueness (47.6 bits).
name = '__libc_early_init_'
for n in range(8):
    name += digits[derived_bits % len(digits)]
    derived_bits //= len(digits)

# Write the output file.
with open(opts.output, 'w') if opts.output else sys.stdout as out:
    out.write('#define LIBC_EARLY_INIT_NAME {}\n'.format(name))
    out.write('#define LIBC_EARLY_INIT_NAME_STRING "{}"\n'.format(name))
    out.write('#define LIBC_EARLY_INIT_GNU_HASH {}\n'.format(
        gnu_hash(name)))
