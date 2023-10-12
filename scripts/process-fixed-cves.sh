#!/bin/bash -e
# Copyright The GNU Toolchain Authors.
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

if ! [ -d advisories ]; then
  echo "error: Run me from the toplevel directory of the glibc repository."
  exit 1
fi

release=$(echo RELEASE | gcc -E -include version.h -o - - | grep -v "^#")
minor=$(echo __GLIBC_MINOR__ | gcc -E -include include/features.h -o - - |
	grep -v "^#")

if [ $release = "\"development\"" ]; then
  cur_rel=2.$((minor + 1))
else
  cur_rel=2.$minor
fi

for f in $(grep -l "^Fix-Commit: .* ($cur_rel)$" advisories/*); do
  echo -e "  $(basename $f):"
  cve_id=$(sed -n 's/CVE-Id: \(.*\)/\1/p' $f)
  echo "$(head -1 $f) ($cve_id)" | fold -w 68 -s | while read line; do
    echo "    $line"
  done
  echo
done
