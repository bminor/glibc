# Makefile for the GNU C Library manual.

# Copyright (C) 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

.PHONY: all
all: library.dvi library.info

chapters: library.texinfo Makefile
	(echo 'define chapters'				;\
	 sed -n 's/^@include *\(.*\)$$/\1/p' < $<	;\
	 echo 'endef') > $@.new
	 mv $@.new $@
include chapters
define \n


endef
chapters := $(subst $(\n), ,$(chapters))

library.dvi library.info: $(chapters) summary.texinfo texinfo.tex

summary.texinfo: summary.awk $(chapters)
	awk -f $^ \
	| sort +1 -2 | tr '\014' '\012' > $@.new
	mv $@.new $@

ifeq (,)
headers :=
sources :=
subdir := manual
distribute := library.texinfo $(chapters) texinfo.tex \
	      summary.texinfo summary.awk
.PHONY: dist
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)

include ../Makeconfig
endif
