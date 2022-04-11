#!/usr/bin/python3
# Verify scripts/glibcelf.py contents against elf/elf.h.
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

import argparse
import enum
import sys

import glibcelf
import glibcextract

errors_encountered = 0

def error(message):
    global errors_encountered
    sys.stdout.write('error: {}\n'.format(message))
    errors_encountered += 1

# The enum constants in glibcelf are expected to have exactly these
# prefixes.
expected_constant_prefixes = tuple(
    'ELFCLASS ELFDATA EM_ ET_ DT_ PF_ PT_ SHF_ SHN_ SHT_ STB_ STT_'.split())

def find_constant_prefix(name):
    """Returns a matching prefix from expected_constant_prefixes or None."""
    for prefix in expected_constant_prefixes:
        if name.startswith(prefix):
            return prefix
    return None

def find_enum_types():
    """A generator for OpenIntEnum and IntFlag classes in glibcelf."""
    for obj in vars(glibcelf).values():
        if isinstance(obj, type) and obj.__bases__[0] in (
                glibcelf._OpenIntEnum, enum.Enum, enum.IntFlag):
            yield obj

def check_duplicates():
    """Verifies that enum types do not have duplicate values.

    Different types must have different member names, too.

    """
    global_seen = {}
    for typ in find_enum_types():
        seen = {}
        last = None
        for (name, e) in typ.__members__.items():
            if e.value in seen:
                error('{} has {}={} and {}={}'.format(
                    typ, seen[e.value], e.value, name, e.value))
                last = e
            else:
                seen[e.value] = name
                if last is not None and last.value > e.value:
                    error('{} has {}={} after {}={}'.format(
                        typ, name, e.value, last.name, last.value))
                if name in global_seen:
                    error('{} used in {} and {}'.format(
                        name, global_seen[name], typ))
                else:
                    global_seen[name] = typ

def check_constant_prefixes():
    """Check that the constant prefixes match expected_constant_prefixes."""
    seen = set()
    for typ in find_enum_types():
        typ_prefix = None
        for val in typ:
            prefix = find_constant_prefix(val.name)
            if prefix is None:
                error('constant {!r} for {} has unknown prefix'.format(
                    val, typ))
                break
            elif typ_prefix is None:
                typ_prefix = prefix
                seen.add(typ_prefix)
            elif prefix != typ_prefix:
                error('prefix {!r} for constant {!r}, expected {!r}'.format(
                    prefix, val, typ_prefix))
        if typ_prefix is None:
            error('empty enum type {}'.format(typ))

    for prefix in sorted(set(expected_constant_prefixes) - seen):
        error('missing constant prefix {!r}'.format(prefix))
    # Reverse difference is already covered inside the loop.

def find_elf_h_constants(cc):
    """Returns a dictionary of relevant constants from <elf.h>."""
    return glibcextract.compute_macro_consts(
        source_text='#include <elf.h>',
        cc=cc,
        macro_re='|'.join(
            prefix + '.*' for prefix in expected_constant_prefixes))

# The first part of the pair is a name of an <elf.h> constant that is
# dropped from glibcelf.  The second part is the constant as it is
# used in <elf.h>.
glibcelf_skipped_aliases = (
    ('EM_ARC_A5', 'EM_ARC_COMPACT'),
    ('PF_PARISC_SBP', 'PF_HP_SBP')
)

# Constants that provide little value and are not included in
# glibcelf: *LO*/*HI* range constants, *NUM constants counting the
# number of constants.  Also includes the alias names from
# glibcelf_skipped_aliases.
glibcelf_skipped_constants = frozenset(
    [e[0] for e in glibcelf_skipped_aliases]) | frozenset("""
DT_AARCH64_NUM
DT_ADDRNUM
DT_ADDRRNGHI
DT_ADDRRNGLO
DT_ALPHA_NUM
DT_ENCODING
DT_EXTRANUM
DT_HIOS
DT_HIPROC
DT_IA_64_NUM
DT_LOOS
DT_LOPROC
DT_MIPS_NUM
DT_NUM
DT_PPC64_NUM
DT_PPC_NUM
DT_PROCNUM
DT_SPARC_NUM
DT_VALNUM
DT_VALRNGHI
DT_VALRNGLO
DT_VERSIONTAGNUM
ELFCLASSNUM
ELFDATANUM
ET_HIOS
ET_HIPROC
ET_LOOS
ET_LOPROC
ET_NUM
PF_MASKOS
PF_MASKPROC
PT_HIOS
PT_HIPROC
PT_HISUNW
PT_LOOS
PT_LOPROC
PT_LOSUNW
SHF_MASKOS
SHF_MASKPROC
SHN_HIOS
SHN_HIPROC
SHN_HIRESERVE
SHN_LOOS
SHN_LOPROC
SHN_LORESERVE
SHT_HIOS
SHT_HIPROC
SHT_HIPROC
SHT_HISUNW
SHT_HIUSER
SHT_LOOS
SHT_LOPROC
SHT_LOSUNW
SHT_LOUSER
SHT_NUM
STB_HIOS
STB_HIPROC
STB_LOOS
STB_LOPROC
STB_NUM
STT_HIOS
STT_HIPROC
STT_LOOS
STT_LOPROC
STT_NUM
""".strip().split())

def check_constant_values(cc):
    """Checks the values of <elf.h> constants against glibcelf."""

    glibcelf_constants = {
        e.name: e for typ in find_enum_types() for e in typ}
    elf_h_constants = find_elf_h_constants(cc=cc)

    missing_in_glibcelf = (set(elf_h_constants) - set(glibcelf_constants)
                           - glibcelf_skipped_constants)
    for name in sorted(missing_in_glibcelf):
        error('constant {} is missing from glibcelf'.format(name))

    unexpected_in_glibcelf = \
        set(glibcelf_constants) & glibcelf_skipped_constants
    for name in sorted(unexpected_in_glibcelf):
        error('constant {} is supposed to be filtered from glibcelf'.format(
            name))

    missing_in_elf_h = set(glibcelf_constants) - set(elf_h_constants)
    for name in sorted(missing_in_elf_h):
        error('constant {} is missing from <elf.h>'.format(name))

    expected_in_elf_h = glibcelf_skipped_constants - set(elf_h_constants)
    for name in expected_in_elf_h:
        error('filtered constant {} is missing from <elf.h>'.format(name))

    for alias_name, name_in_glibcelf in glibcelf_skipped_aliases:
        if name_in_glibcelf not in glibcelf_constants:
            error('alias value {} for {} not in glibcelf'.format(
                name_in_glibcelf, alias_name))
        elif (int(elf_h_constants[alias_name])
              != glibcelf_constants[name_in_glibcelf].value):
            error('<elf.h> has {}={}, glibcelf has {}={}'.format(
                alias_name, elf_h_constants[alias_name],
                name_in_glibcelf, glibcelf_constants[name_in_glibcelf]))

    # Check for value mismatches:
    for name in sorted(set(glibcelf_constants) & set(elf_h_constants)):
        glibcelf_value = glibcelf_constants[name].value
        elf_h_value = int(elf_h_constants[name])
        # On 32-bit architectures <elf.h> as some constants that are
        # parsed as signed, while they are unsigned in glibcelf.  So
        # far, this only affects some flag constants, so special-case
        # them here.
        if (glibcelf_value != elf_h_value
            and not (isinstance(glibcelf_constants[name], enum.IntFlag)
                     and glibcelf_value == 1 << 31
                     and elf_h_value == -(1 << 31))):
            error('{}: glibcelf has {!r}, <elf.h> has {!r}'.format(
                name, glibcelf_value, elf_h_value))

def main():
    """The main entry point."""
    parser = argparse.ArgumentParser(
        description="Check glibcelf.py and elf.h against each other.")
    parser.add_argument('--cc', metavar='CC',
                        help='C compiler (including options) to use')
    args = parser.parse_args()

    check_duplicates()
    check_constant_prefixes()
    check_constant_values(cc=args.cc)

    if errors_encountered > 0:
        print("note: errors encountered:", errors_encountered)
        sys.exit(1)

if __name__ == '__main__':
    main()
