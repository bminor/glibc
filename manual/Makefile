# Makefile for the GNU C Library manual.

# Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

.PHONY: all dvi info
all: dvi info
dvi: library.dvi
info: library.info

# Set chapters and chapters-incl.
include chapters
chapters: library.texinfo
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
summary.texinfo: stamp-summary ;
stamp-summary: summary.awk $(chapters)
	awk -f $^ \
	| sort +1 -2 | tr '\014' '\012' > summary-tmp
	move-if-change summary-tmp summary.texinfo
	touch $@

# Generate Texinfo files from the C source for the example programs.
%.c.texi: examples/%.c
	sed -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\* *\(.*\) *\*/,/* @r{\1} */,g'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/'\
	    $< > $@.new
	mv $@.new $@


minimal-dist = COPYING.LIB summary.awk Makefile move-if-change		\
	       library.texinfo $(chapters)				\
	       $(patsubst %.c.texi,examples/%.c,			\
			  $(filter-out summary.texinfo,$(chapters-incl)))
distribute = $(minimal-dist)	       					\
	     $(patsubst examples/%.c,%.c.texi,$(filter examples/%.c,	\
			$(minimal-dist)))				\
	     library.?? library.??s texinfo.tex summary.texinfo		\
	     stamp-summary chapters chapters-incl

tar-it = tar chvf $@ $^

manual.tar: $(minimal-dist) ; $(tar-it)
mandist.tar: $(distribute) ; $(tar-it)

edition := $(shell sed -n 's/^@set EDITION \(.*\)$$/\1/p' library.texinfo)

glibc-manual-$(edition).tar: $(distribute)
	@rm -f glibc-manual-$(edition)
	ln -s . glibc-manual-$(edition)
	tar chvf $@ $(addprefix glibc-manual-$(edition)/,$^)
	rm -f glibc-manual-$(edition)

%.Z: %
	compress -c $< > $@.new
	mv $@.new $@
%.z: %
	gzip -9 -c $< > $@.new
	mv $@.new $@
%.uu: %
	uuencode $< < $< > $@.new
	mv $@.new $@

.PHONY: dist
dist: $(addprefix glibc-manual-$(edition).tar,.z .Z)

ifneq (,)
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
endif

ifndef ETAGS
ETAGS = etags -T
endif
TAGS: $(minimal-dist)
	$(ETAGS) -o $@ $^
