#!/bin/sh
# Test --preload argument ld.so.
# Copyright (C) 2019 Free Software Foundation, Inc.
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

set -e

rtld=$1
test_program=$2
test_wrapper=$3
test_wrapper_env=$4
run_program_env=$5
library_path=$6
preload=$7

echo "# [${test_wrapper}] [$rtld] [--library-path] [$library_path]" \
     "[--preload] [$preload] [$test_program]"
${test_wrapper_env} \
${run_program_env} \
${test_wrapper} $rtld --library-path "$library_path" \
  --preload "$preload" $test_program 2>&1 && rc=0 || rc=$?
echo "# exit status $rc"

exit $rc
