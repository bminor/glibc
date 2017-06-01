#!/usr/bin/python3
# Construct a build tree expressing sysdeps overrides.
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

"""Construct a build tree expressing sysdeps overrides.

This script is run automatically as the first action in any 'make'
invocation.  It scans the complete glibc source tree to determine
which files should be used in this configuration.  From that, it
generates or updates three directories and one Makefile fragment in
the current working directory:

   sources.mk   - Makefile fragment mapping object to source files
   include      - contains all header files that will be installed
   include-wrap - contains all internal wrappers of installed header files
   include-int  - contains all internal-only header files

The header files are not copied into the generated include directories;
rather, shim headers (containing only '#include <path/to/real-header.h>')
are generated.

"""

import argparse
import os
import sys

def parse_args():
    """Parse and return the command-line arguments."""

    def ensure_directory(path):
        if not os.path.isdir(path):
            raise argparse.ArgumentTypeError(
                "{!r}: not a directory".format(path))
        return path

    def comma_list(lst):
        return [x for x in lst.split(",") if x]

    global __doc__
    doc = __doc__.splitlines()

    ap = argparse.ArgumentParser(
        description=doc[0],
        epilog="\n".join(doc[1:]),
        formatter_class=argparse.RawDescriptionHelpFormatter)

    ap.add_argument("--srcdir",
                    help="Path to glibc source tree.",
                    type=ensure_directory,
                    default=os.path.dirname(os.path.dirname(__file__)))
    ap.add_argument("--suffixes",
                    help="Comma-separated list of object file suffixes.",
                    metavar="o,os,...",
                    type=comma_list,
                    default="o")

    return ap.parse_args()

def prune_sysdirs(dnames, path):
    """Return the subset of DNAMES (all of which are subdirectories of
       PATH) which are *not* themselves sysdeps directories.

       A directory is a sysdeps directory if it _could_ appear in the
       sysdirs list on some configuration.  The logic below is
       heuristic.  It is more important for it to have no false
       positives (directories identified as sysdeps when they aren't),
       which will merely generate useless wrapper headers and/or
       sources.mk entries, than false negatives, which may cause
       files to be left out of the build entirely.
    """
    pruned = []
    PJ = os.path.join
    EX = os.path.isfile
    for d in dnames:
        if d in ('include', 'fpu', 'nofpu', 'multiarch'):
            continue
        p = PJ(path, d)
        # This list is sorted in decreasing order of frequency of use.
        for rf in ('Implies', 'Makefile', 'Versions', 'sysdep.h',
                   'configure.ac', 'configure', 'preconfigure',
                   'Subdirs', 'Makeconfig', 'preconfigure.ac',
                   'Implies-after'):
            if EX(PJ(p, rf)):
                break
        else:
            pruned.append(d)

    return pruned

def extract_value_from_makefile(variable, makefile):
    """Parse MAKEFILE and return the apparent value of VARIABLE.
       This uses a crude approximation to the Makefile grammar and
       does not attempt to process $(anything)."""
    with open(makefile, 'rt') as fp:
        return extract_value_from_makefile_inner(variable, makefile, fp)

def extract_value_from_makefile_inner(variable, makefile, fp):
    """Parse MAKEFILE and return the apparent value of VARIABLE -
       subroutine that does the real work."""
    continued = False
    in_define = False
    logical_line = ""
    logical_lineno = 0
    value = []

    for lineno, line in enumerate(fp):
        line = line.rstrip()
        if continued:
            logical_line += line
        else:
            logical_lineno = lineno
            logical_line = line

        if not logical_line:
            continued = False
            continue

        # FIXME: Doesn't handle double backslash at end of line.
        if logical_line[-1] == '\\':
            continued = True
            logical_line = logical_line[:-1] + ' '
            continue

        # If we get here, 'logical_line' is a complete and nonempty
        # logical line.
        continued = False

        # Ignore recipe lines.
        if logical_line[0] == '\t':
            continue

        # Ignore blank lines and comments.
        logical_line = logical_line.strip()
        if not logical_line or logical_line[0] == '#':
            continue

        if in_define:
            if logical_line == 'endef':
                in_define = False
            continue

        tokens = logical_line.split()
        if tokens[0] == 'override':
            del tokens[0]

        if tokens[0] == 'define':
            if tokens[1] == variable:
                raise RuntimeError(
                    "{}:{}: sorry, not implemented: {} {} ..."
                    .format(makefile, logical_lineno,
                            tokens[0], tokens[1]))
            in_define = True
            continue

        if tokens[0] == variable:
            # We generally want all of the tokens that this Makefile
            # _could_ put into this variable, even if they aren't always.
            # So we ignore which type of assignment it is.
            # This is also why we don't bother parsing conditionals.
            if not (tokens[1] == '=' or
                    tokens[1] == ':=' or
                    tokens[1] == '::=' or
                    tokens[1] == '+='):
                raise RuntimeError(
                    "{}: sorry, not implemented: {} {} ..."
                    .format(tokens[0], tokens[1]))
            for token in tokens[2:]:
                if token[0] == '$':
                    raise RuntimeError(
                        "{}:{} sorry, not implemented: "
                        "$(...) in {}"
                        .format(makefile, logical_lineno, variable))
                value.append(token)

    return value


class SrcSubdir:
    """Represents one subdirectory of the source tree, either at top
       level or within the sysdeps hierarchy."""

    def __init__(self, subdir, srcdir):
        self.path = os.path.join(srcdir, subdir)
        self.sources  = {}
        self.xheaders = set()
        self.wheaders = set()
        self.iheaders = set()

        def walk_error(e): raise e
        for dpath, dnames, fnames in os.walk(self.path,
                                             topdown=True,
                                             onerror=walk_error):
            reltop = os.path.relpath(dpath, self.path)
            if reltop == '.': reltop = ''
            for fn in fnames:
                if fn.endswith('.h'):
                    self.iheaders.add(os.path.join(reltop, fn))
                elif (fn.endswith('.c') or
                      fn.endswith('.cc') or
                      fn.endswith('.s') or
                      fn.endswith('.S')):
                    path = os.path.join(reltop, fn)
                    prefix = os.path.splitext(path)[0]
                    self.sources[prefix] = path

            if subdir.startswith('sysdeps/'):
                dnames[:] = prune_sysdirs(dnames, self.path)

        wrapperdir = os.path.join(self.path, 'include')
        if os.path.isdir(wrapperdir):
            self.process_wrapper_subdir(wrapperdir)
        self.process_Makefile(os.path.join(self.path, 'Makefile'))

    def process_wrapper_subdir(self, wrapperdir):
        """If a sysdeps directory contains a directory named 'include',
           that directory contains wrapper headers."""
        for dpath, dnames, fnames in os.walk(wrapperdir):
            reltop = os.path.relpath(dpath, self.path)
            if reltop == '.': reltop = ''
            for fn in fnames:
                if fn.endswith('.h'):
                    self.wheaders.add(os.path.join(reltop, fn))

    def process_Makefile(self, makefile):
        """Parse the Makefile in this directory to figure out which
           headers are installed and which are internal.  This uses
           a crude approximation to gnumake syntax and doesn't even
           try to evaluate $(anything)."""

        try:
            xheaders = set(extract_value_from_makefile('headers', makefile))
            xheaders |= set(extract_value_from_makefile('sysdep_headers', makefile))
        except (FileNotFoundError, NotADirectoryError):
            xheaders = set()

        self.iheaders -= xheaders
        self.xheaders = xheaders

    def special_handling_for_include_dir(self):
        """The top-level include directory is special: it contains
           external headers, wrapper headers, and internal-use-only
           headers, but no Makefile to tell us which is which."""
        pass

def write_srcdep(f, d, s, sp, suffixes):
    p = os.path.join(d, s)
    for o in suffixes:
        f.write("{}.{}: {}\n".format(p, o, sp))

def write_sources_mk(subdirs, sysd_subdirs, suffixes):
    """Write out sources.mk, listing which source file to build each
       object file from.
    """
    handled = set()
    with open('sources.mk.T', 'wt') as f:
        for d, data in subdirs:
            first = True
            for s in sorted(data.sources.keys()):
                if first:
                    f.write("# {}\n".format(d))
                    first = False
                handled.add(s)
                for _, sd in sysd_subdirs:
                    if s in sd.sources:
                        write_srcdep(f, d, s,
                                     os.path.join(sd.path, sd.sources[s]),
                                     suffixes)
                        break
                else:
                    write_srcdep(f, d, s,
                                 os.path.join(data.path, data.sources[s]),
                                 suffixes)
            if not first:
                f.write("\n")

        for d, sd in sysd_subdirs:
            first = True
            for s in sorted(sd.sources.keys()):
                if s not in handled:
                    handled.add(s)
                    if first:
                        f.write("# {}\n".format(d))
                        first = False
                    write_srcdep(f, d, s,
                                 os.path.join(sd.path, sd.sources[s]),
                                 suffixes)
            if not first:
                f.write("\n")

def main():
    args = parse_args()

    # Top-level directories are not ordered.
    top_level_subdirs = sorted(
        (d, SrcSubdir(d, args.srcdir))
        for d in extract_value_from_makefile('sorted-subdirs', 'sysd-sorted')
    )

    # Sysdeps directories are ordered.
    sysdeps_dirs = [
        (d, SrcSubdir(d, args.srcdir))
        for d in extract_value_from_makefile('config-sysdirs', 'config.make')
    ]

    # The top-level 'include' directory is special.
    include_dir = SrcSubdir('include', args.srcdir)
    include_dir.special_handling_for_include_dir()

    write_sources_mk(top_level_subdirs, sysdeps_dirs, args.suffixes)

main()
