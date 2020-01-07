# Git repo support.
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

from gitlog_to_changelog import analyze_diff
import subprocess
import re
from misc_util import *

class GitRepo:
    def __init__(self, ignore_list, debug):
        self.ignore_list = ignore_list
        self.debug = debug


    def exec_git_cmd(self, args):
        ''' Execute a git command and return its result as a list of strings.
        '''
        args.insert(0, 'git')
        self.debug.print(args)
        proc = subprocess.Popen(args, stdout=subprocess.PIPE)

        # Clean up the output by removing trailing spaces, newlines and dropping
        # blank lines.
        op = [decode(x[:-1]).strip() for x in proc.stdout]
        op = [re.sub(r'[\s\f]+', ' ', x) for x in op]
        op = [x for x in op if x]
        return op


    def list_changes(self, commit, frontends):
        ''' List changes in a single commit.

        For the input commit id COMMIT, identify the files that have changed and the
        nature of their changes.  Print commit information in the ChangeLog format,
        calling into helper functions as necessary.
        '''

        op = self.exec_git_cmd(['show', '--pretty=fuller', '--date=short',
                                '--raw', commit])
        authors = []
        date = ''
        merge = False
        copyright_exempt=''
        subject= ''

        for l in op:
            if l.lower().find('copyright-paperwork-exempt:') == 0 \
                    and 'yes' in l.lower():
                copyright_exempt=' (tiny change)'
            elif l.lower().find('co-authored-by:') == 0 or \
                    l.find('Author:') == 0:
                author = l.split(':')[1]
                author = re.sub(r'([^ ]*)\s*(<.*)', r'\1  \2', author.strip())
                authors.append(author)
            elif l.find('CommitDate:') == 0:
                date = l[11:].strip()
            elif l.find('Merge:') == 0:
                merge = True
            elif not subject and date:
                subject = l.strip()

        # Find raw commit information for all non-ChangeLog files.
        op = [x[1:] for x in op if len(x) > 0 and re.match(r'^:[0-9]+', x)]

        # Skip all ignored files.
        for ign in self.ignore_list:
            op = [x for x in op if ign not in x]

        # It was only the ChangeLog, ignore.
        if len(op) == 0:
            return

        print('%s  %s' % (date, authors[0]))

        if (len(authors) > 1):
            authors = authors[1:]
            for author in authors:
                print('            %s' % author)

        print()

        if merge:
           print('\t MERGE COMMIT: %s\n' % commit)
           return

        print('\tCOMMIT%s: %s\n\t%s\n' % (copyright_exempt, commit, subject))

        # Changes across a large number of files are typically mechanical (URL
        # updates, copyright notice changes, etc.) and likely not interesting
        # enough to produce a detailed ChangeLog entry.
        if len(op) > 100:
            print('\t* Suppressing diff as too many files differ.\n')
            return

        # Each of these lines has a space separated format like so:
        # :<OLD MODE> <NEW MODE> <OLD REF> <NEW REF> <OPERATION> <FILE1> <FILE2>
        #
        # where OPERATION can be one of the following:
        # A: File added
        # D: File removed
        # M[0-9]{3}: File modified
        # R[0-9]{3}: File renamed, with the 3 digit number following it indicating
        # what percentage of the file is intact.
        # C[0-9]{3}: File copied.  Same semantics as R.
        # T: The permission bits of the file changed
        # U: Unmerged.  We should not encounter this, so we ignore it/
        # X, or anything else: Most likely a bug.  Report it.
        #
        # FILE2 is set only when OPERATION is R or C, to indicate the new file name.
        #
        # Also note that merge commits have a different format here, with three
        # entries each for the modes and refs, but we don't bother with it for now.
        #
        # For more details: https://git-scm.com/docs/diff-format
        for f in op:
            data = f.split()
            if data[4] == 'A':
                print('\t* %s: New file.' % data[5])
            elif data[4] == 'D':
                print('\t* %s: Delete file.' % data[5])
            elif data[4] == 'T':
                print('\t* %s: Changed file permission bits from %s to %s' % \
                        (data[5], data[0], data[1]))
            elif data[4][0] == 'M':
                print('\t* %s: Modified.' % data[5])
                analyze_diff(data[5],
                             self.exec_git_cmd(['show', data[2]]),
                             self.exec_git_cmd(['show', data[3]]), frontends)
            elif data[4][0] == 'R' or data[4][0] == 'C':
                change = int(data[4][1:])
                print('\t* %s: Move to...' % data[5])
                print('\t* %s: ... here.' % data[6])
                if change < 100:
                    analyze_diff(data[6],
                                 self.exec_git_cmd(['show', data[2]]),
                                 self.exec_git_cmd(['show', data[3]]), frontends)
            # We should never encounter this, so ignore for now.
            elif data[4] == 'U':
                pass
            else:
                eprint('%s: Unknown line format %s' % (commit, data[4]))
                sys.exit(42)

        print('')


    def list_commits(self, revs):
        ''' List commit IDs between the two revs in the REVS list.
        '''
        ref = revs[0] + '..' + revs[1]
        return self.exec_git_cmd(['log', '--pretty=%H', ref])
