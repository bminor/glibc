# Copyright (C) 2003-2013 Free Software Foundation, Inc.
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
# <http://www.gnu.org/licenses/>.

# Makefile for google-nsl-stub add-on subdirectory of GNU C Library.
# Based on libidn/Makefile.

subdir	:= google-nsl-stub

extra-libs		= libnsl
extra-libs-others	= $(extra-libs)

libnsl-routines := ypclnt

include $(..)Makeconfig

libnsl-inhibit-o = $(filter-out .os,$(object-suffixes))

include $(..)Rules
