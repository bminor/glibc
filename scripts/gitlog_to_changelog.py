#!/usr/bin/python3
# Main VCSToChangeLog script.
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

''' Generate a ChangeLog style output based on a VCS log.

This script takes two revisions as input and generates a ChangeLog style output
for all revisions between the two revisions.

This script is intended to be executed from the project parent directory.

The vcs_to_changelog directory has a file vcstocl_quirks.py that defines a
function called get_project_quirks that returns a object of class type
ProjectQuirks or a subclass of the same.  The definition of the ProjectQuirks
class is below and it specifies the properties that the project must set to
ensure correct parsing of its contents.

Among other things, ProjectQurks specifies the VCS to read from; the default is
assumed to be git.  The script then studies the VCS log and for each change,
list out the nature of changes in the constituent files.

Each file type may have parser frontends that can read files and construct
objects that may be compared to determine the minimal changes that occured in
each revision.  For files that do not have parsers, we may only know the nature
of changes at the top level depending on the information that the VCS stores.

The parser frontend must have a compare() method that takes the old and new
files as arrays of strings and prints the output in ChangeLog format.

Currently implemented VCS:

    git

Currently implemented frontends:

    C
'''
import sys
import os
import re
import argparse
from vcs_to_changelog.misc_util import *
from vcs_to_changelog import frontend_c
from vcs_to_changelog.vcs_git import *

debug = DebugUtil(False)

class ProjectQuirks:
    # This is a list of regex substitutions for C/C++ macros that are known to
    # break parsing of the C programs.  Each member of this list is a dict with
    # the key 'orig' having the regex and 'sub' having the substitution of the
    # regex.
    MACRO_QUIRKS = []

    # This is a list of macro definitions that are extensively used and are
    # known to break parsing due to some characteristic, mainly the lack of a
    # semicolon at the end.
    C_MACROS = []

    # The repo type, defaults to git.
    repo = 'git'

    # List of files to ignore either because they are not needed (such as the
    # ChangeLog) or because they are non-parseable.  For example, glibc has a
    # header file that is only assembly code, which breaks the C parser.
    IGNORE_LIST = ['ChangeLog']


# Load quirks file.  We assume that the script is run from the top level source
# directory.
sys.path.append('/'.join([os.getcwd(), 'scripts', 'vcs_to_changelog']))
try:
    from vcstocl_quirks import *
    project_quirks = get_project_quirks(debug)
except:
    project_quirks = ProjectQuirks()

def analyze_diff(filename, oldfile, newfile, frontends):
    ''' Parse the output of the old and new files and print the difference.

    For input files OLDFILE and NEWFILE with name FILENAME, generate reduced
    trees for them and compare them.  We limit our comparison to only C source
    files.
    '''
    name, ext = os.path.splitext(filename)

    if not ext in frontends.keys():
        return None
    else:
        frontend = frontends[ext]
        frontend.compare(oldfile, newfile)


def main(repo, frontends, refs):
    ''' ChangeLog Generator Entry Point.
    '''
    commits = repo.list_commits(args.refs)
    for commit in commits:
        repo.list_changes(commit, frontends)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('refs', metavar='ref', type=str, nargs=2,
                     help='Refs to print ChangeLog entries between')

    parser.add_argument('-d', '--debug', required=False, action='store_true',
                     help='Run the file parser debugger.')

    args = parser.parse_args()

    debug.debug = args.debug

    if len(args.refs) < 2:
        debug.eprint('Two refs needed to get a ChangeLog.')
        sys.exit(os.EX_USAGE)

    REPO = {'git': GitRepo(project_quirks.IGNORE_LIST, debug)}

    fe_c = frontend_c.Frontend(project_quirks, debug)
    FRONTENDS = {'.c': fe_c,
                 '.h': fe_c}

    main(REPO[project_quirks.repo], FRONTENDS, args.refs)
