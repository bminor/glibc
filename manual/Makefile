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

subdir := manual

.PHONY: all
all: library.dvi library.info

# Set chapters and chapters-incl.
include chapters
chapters: library.texinfo Makefile
	$(find-includes)
ifdef chapters
include chapters-incl
chapters-incl: $(chapters)
	$(find-includes)
endif

define find-includes
(echo '$@ :=' \\	;\
 awk '$$1 == "@include" { print $$2 " \\" }' $^) > $@.new
mv $@.new $@
endef

library.dvi library.info: $(chapters) summary.texinfo $(chapters-incl)
library.dvi: texinfo.tex

# Generate the summary from the Texinfo source files for each chapter.
summary.texinfo: summary.awk $(chapters)
	awk -f $^ \
	| sort +1 -2 | tr '\014' '\012' > $@.new
	mv $@.new $@

# Generate Texinfo files from the C source for the example programs.
%.c.texi: examples/%.c
	sed -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\* *\(.*\) *\*/,/* @r{\1} */,g'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/'\
	    $< > $@.new
	mv $@.new $@


minimal-dist = library.texinfo summary.awk Makefile	\
	       $(chapters) $(chapters-incl:%.c.texi=examples/%.c)
distribute = $(minimal-dist) texinfo.tex summary.texinfo

manual.tar: $(minimal-dist)
	tar uvf $@ $^

%.Z: %
	compress -c $< > $@.new
	mv $@.new $@

%.uu: %
	uuencode $< < $< > $@.new
	mv $@.new $@

ifneq (,)
.PHONY: dist
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
endif
