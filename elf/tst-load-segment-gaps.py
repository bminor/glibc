#!/usr/bin/python3
# Verify that objects do not contain gaps in load segments.
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

import argparse
import os.path
import sys

# Make available glibc Python modules.
sys.path.append(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), os.path.pardir, 'scripts'))

import glibcelf

def rounddown(val, align):
    assert (align & (align - 1)) == 0, align
    return val & -align
def roundup(val, align):
    assert (align & (align - 1)) == 0, align
    return (val + align - 1) & -align

errors = False

def process(path, img):
    global errors
    loads = [phdr for phdr in img.phdrs()
             if phdr.p_type == glibcelf.Pt.PT_LOAD]
    if not loads:
        # Nothing ot check.
        return
    alignments = [phdr.p_align for phdr in loads if phdr.p_align > 0]
    if alignments:
        align = min(alignments)
    else:
        print('error: cannot infer page size')
        errors = True
        align = 4096
    print('info: inferred page size:', align)
    current_address = None
    for idx, phdr in enumerate(loads):
        this_address = rounddown(phdr.p_vaddr, align)
        next_address = roundup(phdr.p_vaddr + phdr.p_memsz, align)
        print('info: LOAD segment {}: address 0x{:x}, size {},'
              ' range [0x{:x},0x{:x})'
               .format(idx, phdr.p_vaddr, phdr.p_memsz,
                       this_address, next_address))
        if current_address is not None:
            gap = this_address - current_address
            if gap != 0:
                errors = True
                print('error: gap between load segments: {} bytes'.format(gap))
        current_address = next_address

for path in sys.argv[1:]:
    img = glibcelf.Image.readfile(path)
    process(path, img)

if errors:
    sys.exit(1)
