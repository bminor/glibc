# Copyright (C) 1991, 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for ctype portion of the library.
#
subdir	:= ctype

routines	:= ctype ctype-extn
aux		:= ctype-info

tests	:= test_ctype

# In most cases, we want the C source files to come before
# the header files so tags for optimizing #define's in the
# headers won't be put in the tags files, but for ctype,
# the functions are just backup for the #define's in the header.
export tags_sources = $(all-headers) $(all-sources)

all:
include ../Rules
