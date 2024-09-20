#!/bin/bash
# Test for iconv (the program) buffer management.
# Copyright (C) 2024 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

exec 2>&1
set -e

exec {logfd}>&1

codir=$1
test_program_prefix="$2"

# Use internal converters to avoid issues with module loading.
iconv_args="-f ASCII -t UTF-8"

failure=false

tmp=`mktemp -d`
trap 'rm -rf "$tmp"' 0
echo ABC > "$tmp/abc"
echo DEF > "$tmp/def"
echo GGG > "$tmp/ggg"
echo HH > "$tmp/hh"
echo XY > "$tmp/xy"
echo ZT > "$tmp/zt"
echo OUT > "$tmp/out-template"
printf '\xff' > "$tmp/0xff"
cat "$tmp/xy" "$tmp/0xff" "$tmp/zt" > "$tmp/0xff-wrapped"

run_iconv () {
    local c=0
    if test "${FUNCNAME[2]}" = main; then
       c=1
    fi
    echo "${BASH_SOURCE[$c]}:${BASH_LINENO[$c]}: iconv $iconv_args $@" >&$logfd
    $test_program_prefix $codir/iconv/iconv_prog $iconv_args "$@"
}

check_out_expected () {
    if ! cmp -s "$tmp/out" "$tmp/expected" ; then
        echo "error: iconv output difference" >&$logfd
        echo "*** expected ***" >&$logfd
        cat "$tmp/expected" >&$logfd
        echo "*** actual ***" >&$logfd
        cat "$tmp/out" >&$logfd
        failure=true
    fi
}

expect_files () {
    local f
    ! test -z "$1"
    cp "$tmp/$1" "$tmp/expected"
    shift
    for f in "$@" ; do
        cat "$tmp/$f" >> "$tmp/expected"
    done
    check_out_expected
}

check_out () {
    cat > "$tmp/expected"
    check_out_expected
}

expect_exit () {
    local expected=$1
    shift
    # Prevent failure for stopping the script.
    if "$@" ; then
        actual=$?
    else
        actual=$?
    fi
    if test "$actual" -ne "$expected"; then
        echo "error: expected exit status $expected, not $actual" >&$logfd
        exit 1
    fi
}

ignore_failure () {
    set +e
    "$@"
    status=$?
    set -e
}

# Concatentation test.
run_iconv -o "$tmp/out" "$tmp/abc" "$tmp/def"
expect_files abc def

# Single-file in-place conversion.
run_iconv -o "$tmp/out" "$tmp/out"
expect_files abc def

# Multiple input files with in-place conversion.

run_iconv -o "$tmp/out" "$tmp/out" "$tmp/abc"
expect_files abc def abc

# But not if we are writing to standard output.

cp "$tmp/out-template" "$tmp/out"
run_iconv </dev/null >>"$tmp/out"
expect_files out-template

cp "$tmp/out-template" "$tmp/out"
run_iconv - </dev/null >>"$tmp/out"
expect_files out-template

cp "$tmp/out-template" "$tmp/out"
run_iconv /dev/null >>"$tmp/out"
expect_files out-template

# Conversion errors should avoid clobbering an existing file if
# it is also an input file.

cp "$tmp/0xff" "$tmp/out"
expect_exit 1 run_iconv -o "$tmp/out" "$tmp/out"
expect_files 0xff

cp "$tmp/0xff" "$tmp/out"
expect_exit 1 run_iconv -o "$tmp/out" < "$tmp/out"
expect_files 0xff

cp "$tmp/0xff" "$tmp/out"
expect_exit 1 run_iconv -o "$tmp/out" - < "$tmp/out"
expect_files 0xff

# If errors are ignored, the file should be overwritten.

cp "$tmp/out-template" "$tmp/out"
expect_exit 1 \
    run_iconv -c -o "$tmp/out" "$tmp/abc" "$tmp/0xff" "$tmp/def" 2>"$tmp/err"
! test -s "$tmp/err"
expect_files abc def

# FIXME: This is not correct, -c should not change the exit status.
cp "$tmp/out-template" "$tmp/out"
run_iconv -c -o "$tmp/out" \
    "$tmp/abc" "$tmp/0xff-wrapped" "$tmp/def" 2>"$tmp/err"
! test -s "$tmp/err"
expect_files abc xy zt def

# If the file does not exist yet, it should not be created on error.

rm "$tmp/out"
expect_exit 1 run_iconv -o "$tmp/out" "$tmp/0xff"
! test -e "$tmp/out"

expect_exit 1 run_iconv -o "$tmp/out" < "$tmp/0xff"
! test -e "$tmp/out"

expect_exit 1 run_iconv -o "$tmp/out" "$tmp/abc" "$tmp/0xff" "$tmp/def"
! test -e "$tmp/out"

expect_exit 1 run_iconv -o "$tmp/out" "$tmp/abc" - < "$tmp/0xff" "$tmp/def"
! test -e "$tmp/out"

if $failure ; then
    exit 1
fi
